#include "Includes.hpp"
#include "GLRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"
#include "prehistoric/core/resources/AssetManager.h"

#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLLightCullingPassShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLHDRShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"

namespace Prehistoric
{
	GLRenderer::GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: Renderer(window, camera, manager), depthFBO{ nullptr }, multisampleFBO{ nullptr },
			colourFBO{ nullptr }, lightBuffer{ nullptr }, visibleLightIndicesBuffer{ nullptr }
	{
		depthFBO = std::make_unique<GLFramebuffer>(window);
		multisampleFBO = std::make_unique<GLFramebuffer>(window);
		colourFBO = std::make_unique<GLFramebuffer>(window);

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		depthImage = GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false);
		colourImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
		outputImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);

		depthFBO->Bind();
		depthFBO->addColourAttachment2D(depthImage);
		depthFBO->Check();
		depthFBO->Unbind();

		multisampleFBO->Bind();
		multisampleFBO->addDepthAttachment(width, height, true);
		multisampleFBO->addColourAttachmentMultisample2D(0);
		multisampleFBO->Check();
		multisampleFBO->Unbind();

		colourFBO->Bind();
		colourFBO->addDepthAttachment(width, height);
		colourFBO->addColourAttachment2D(colourImage);
		colourFBO->Check();
		colourFBO->Unbind();

		AssetManager* man = manager->getAssetManager();

		quad = man->storeVertexBuffer(ModelFabricator::CreateQuad(window));
		quad->setFrontFace(FrontFace::DOUBLE_SIDED);

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		lightCullingShader = man->loadShader(ShaderName::LightCullingPass).value();
		hdrShader = man->loadShader(ShaderName::HDR).value();
		renderShader = man->loadShader(ShaderName::Gui).value();

		lightCullingPipeline = new GLComputePipeline(window, man, lightCullingShader);
		hdrPipeline = new GLComputePipeline(window, man, hdrShader);
		renderPipeline = new GLGraphicsPipeline(window, man, renderShader, quad);

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		lightBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, EngineConfig::lightsMaxNumber * sizeof(InternalLight));
		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

		static_cast<GLComputePipeline*>(lightCullingPipeline)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(lightCullingPipeline)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
		static_cast<GLComputePipeline*>(lightCullingPipeline)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);

		static_cast<GLComputePipeline*>(hdrPipeline)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(hdrPipeline)->addTextureBinding(0, outputImage, WRITE_ONLY);
	}

	GLRenderer::~GLRenderer()
	{
		delete depthImage;
		delete colourImage;
		delete outputImage;
	}

	void GLRenderer::PrepareRendering()
	{
		if (window->isResized())
		{
			PR_PROFILE("Window resize pass");

			uint32_t width = window->getWidth();
			uint32_t height = window->getHeight();

			uint32_t workGroupsX = (width + (width % 16)) / 16;
			uint32_t workGroupsY = (height + (height % 16)) / 16;

			uint32_t numberOfTiles = workGroupsX * workGroupsY;

			visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

			PR_LOG(GREEN, "%d by %d\n", width, height);
			window->getSwapchain()->SetWindowSize(width, height);

			//Recreate the FBO and the images
			delete depthImage;
			delete colourImage;
			delete outputImage;

			depthImage = GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false);
			colourImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
			outputImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);

			static_cast<GLComputePipeline*>(lightCullingPipeline)->removeSSBOBinding(0);
			static_cast<GLComputePipeline*>(lightCullingPipeline)->removeSSBOBinding(1);
			static_cast<GLComputePipeline*>(lightCullingPipeline)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
			static_cast<GLComputePipeline*>(lightCullingPipeline)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
			static_cast<GLComputePipeline*>(lightCullingPipeline)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);

			static_cast<GLComputePipeline*>(hdrPipeline)->removeTextureBinding(0);
			static_cast<GLComputePipeline*>(hdrPipeline)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
			static_cast<GLComputePipeline*>(hdrPipeline)->addTextureBinding(0, outputImage, WRITE_ONLY);

			depthFBO->Bind();
			depthFBO->addColourAttachment2D(depthImage);
			depthFBO->Check();
			depthFBO->Unbind();

			multisampleFBO->Bind();
			multisampleFBO->addDepthAttachment(width, height, true);
			multisampleFBO->addColourAttachmentMultisample2D(0);
			multisampleFBO->Check();
			multisampleFBO->Unbind();

			colourFBO->Bind();
			colourFBO->addDepthAttachment(width, height);
			colourFBO->addColourAttachment2D(colourImage);
			colourFBO->Check();
			colourFBO->Unbind();

			//Recreate the pipelines
			std::vector<Pipeline*> pipes = manager->get<Pipeline>();
			for (Pipeline* pipe : pipes)
			{
				pipe->setViewportSize({ (float)width, (float)height });
				pipe->setScissorSize({ width, height });
				pipe->RecreatePipeline();
			}

			window->setResized(false);
		}
	}

	void GLRenderer::EndRendering()
	{
		window->Render(nullptr);

		//Clear the lists
		models_3d.clear();
		models_transparency.clear();
		models_2d.clear();
		lights.clear();
	}

	void GLRenderer::Render()
	{
		UpdateLightBuffer();

		{
			PR_PROFILE("Depth pass");
			depthFBO->Bind();
			uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
			depthFBO->SetDrawAttachments(1, arr);
			depthFBO->Clear(0.0f);

			depthShader->Bind(nullptr);
			depthShader->UpdateGlobalUniforms(camera, lights);

			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;
				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Bind(nullptr);

				for (auto material : pipeline.second)
				{
					for (auto renderer : material.second)
					{
						depthShader->UpdateObjectUniforms(renderer->getParent());
						pl->RenderPipeline();
					}
				}

				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;
				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Bind(nullptr);

				for (auto material : pipeline.second)
				{
					for (auto renderer : material.second)
					{
						depthShader->UpdateObjectUniforms(renderer->getParent());
						pl->RenderPipeline();
					}
				}

				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
			}

			depthShader->Unbind();
			depthFBO->Unbind();
		}
		
		size_t i = sizeof(InternalLight);

		{
			PR_PROFILE("Light culling pass");

			lightCullingPipeline->BindPipeline(nullptr);
			static_cast<GLLightCullingPassShader*>(lightCullingPipeline->getShader())->UpdateUniforms(camera, lights, depthImage);
			lightCullingPipeline->RenderPipeline();
			lightCullingPipeline->UnbindPipeline();			
		}

		multisampleFBO->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		multisampleFBO->SetDrawAttachments(1, arr);
		multisampleFBO->Clear(0.0f);

		{
			PR_PROFILE("Cubemap pass");
			if (FrameworkConfig::api == OpenGL && EnvironmentMapRenderer::instance && EnvironmentMapRenderer::instance->enabled
				&& EnvironmentMapRenderer::instance->atmosphere == nullptr)
			{
				EnvironmentMapRenderer::instance->RenderCube(camera);
			}
		}

		{
			PR_PROFILE("Main pass");

			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateGlobalUniforms(camera, lights);

				lightBuffer->BindBase(nullptr, 0);
				visibleLightIndicesBuffer->BindBase(nullptr, 1);

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateTextureUniforms(material.first, 0);
					
					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				visibleLightIndicesBuffer->UnbindBase(1);
				lightBuffer->UnbindBase(0);

				pl->UnbindPipeline();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateGlobalUniforms(camera, lights);

				lightBuffer->BindBase(nullptr, 0);
				visibleLightIndicesBuffer->BindBase(nullptr, 1);

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateTextureUniforms(material.first, 0);

					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				visibleLightIndicesBuffer->UnbindBase(1);
				lightBuffer->UnbindBase(0);

				pl->UnbindPipeline();
			}
		}

		multisampleFBO->Unbind();
		multisampleFBO->Blit(colourFBO.get(), window->getWidth(), window->getHeight(), 0, 0);

		//Render using the deferred shader
		{
			PR_PROFILE("HDR post processing");
			hdrPipeline->BindPipeline(nullptr);
			static_cast<GLHDRShader*>(hdrPipeline->getShader())->UpdateUniforms(colourImage);
			hdrPipeline->RenderPipeline();
			hdrPipeline->UnbindPipeline();
		}

		{
			PR_PROFILE("Render to screen");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(outputImage, -1);
			renderPipeline->RenderPipeline();
			renderPipeline->UnbindPipeline();
		}

		//TODO: disable alpha blending and depth testing
		{
			PR_PROFILE("GUI pass");
			for (auto pipeline : models_2d)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);

				for (auto renderer : pipeline.second)
				{
					renderer->BatchRender();
				}

				pl->UnbindPipeline();
			}
		}

	}

	void GLRenderer::UpdateLightBuffer()
	{
		lightBuffer->Bind(nullptr);
		lightBuffer->MapBuffer();
		InternalLight* light = (InternalLight*)lightBuffer->getMappedData();
		memset(light, 0, EngineConfig::lightsMaxNumber * sizeof(InternalLight));

		for (size_t i = 0; i < lights.size(); i++)
		{
			Light* l = lights[i];

			light->position = Vector4f(l->getParent()->getWorldTransform().getPosition(), 1);
			light->colour = Vector4f(l->getColour(), 1);
			light->intensity_radius = Vector4f(l->getIntensity(), l->getRadius());

			light++;
		}
		
		lightBuffer->UnmapBuffer();
		lightBuffer->Unbind();
	}
};
