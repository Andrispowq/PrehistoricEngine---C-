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

namespace Prehistoric
{
	GLRenderer::GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: Renderer(window, camera, manager), depthFBO{nullptr}, lightBuffer{nullptr}, visibleLightIndicesBuffer{nullptr}
	{
		depthFBO = std::make_unique<GLFramebuffer>(window);

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		depthImage = GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false);
		colourImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
		outputImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);

		depthFBO->Bind();
		depthFBO->addColourAttachment2D(depthImage, 0);
		depthFBO->Check();
		depthFBO->Unbind();

		AssetManager* man = manager->getAssetManager();

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		lightCullingShader = man->loadShader(ShaderName::LightCullingPass).value();
		hdrShader = man->loadShader(ShaderName::HDR).value();

		lightCullingPipeline = new GLComputePipeline(window, man, lightCullingShader);
		hdrPipeline = new GLComputePipeline(window, man, hdrShader);

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		lightBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, EngineConfig::lightsMaxNumber * sizeof(InternalLight));
		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex));

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
			depthFBO->addColourAttachment2D(depthImage, 0);
			depthFBO->Check();
			depthFBO->Unbind();

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
		{
			PR_PROFILE("Depth pass");
			depthFBO->Bind();
			uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
			depthFBO->SetDrawAttachments(1, arr);
			depthFBO->Clear(0.0f);

			depthShader->Bind(nullptr);

			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;
				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Bind(nullptr);

				for (auto material : pipeline.second)
				{
					for (auto renderer : material.second)
					{
						renderer->BatchRender();
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
						renderer->BatchRender();
					}
				}
				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
			}

			depthShader->Unbind();
			depthFBO->Unbind();
		}

		{
			PR_PROFILE("Light culling pass");

			lightCullingPipeline->BindPipeline(nullptr);
			static_cast<GLLightCullingPassShader*>(lightCullingPipeline->getShader())->UpdateUniforms(camera, lights, depthImage);
			lightCullingPipeline->RenderPipeline();
			lightCullingPipeline->UnbindPipeline();
			
		}

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

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateTextureUniforms(material.first, 0);
					
					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				pl->UnbindPipeline();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateGlobalUniforms(camera, lights);

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateTextureUniforms(material.first, 0);

					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				pl->UnbindPipeline();
			}
		}

		//Render using the deferred shader
		{
			PR_PROFILE("HDR post processing");
			hdrPipeline->BindPipeline(nullptr);
			static_cast<GLHDRShader*>(hdrPipeline->getShader())->UpdateUniforms(colourImage);
			hdrPipeline->RenderPipeline();
			hdrPipeline->UnbindPipeline();
		}

		//TODO: disable alpha blending and depth testing
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

	void GLRenderer::UpdateLightBuffer()
	{
	}
};
