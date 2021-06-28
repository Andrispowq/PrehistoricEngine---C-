#include "Includes.hpp"
#include "GLRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"
#include "prehistoric/core/resources/AssetManager.h"

#include "platform/opengl/rendering/shaders/deferred/GLAlphaCoverageShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLDeferredShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLFXAAShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"

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

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		lightBuffer = std::make_unique<GLShaderStorageBuffer>(window, 0, EngineConfig::lightsMaxNumber * sizeof(InternalLight));
		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, 0, numberOfTiles * sizeof(VisibleIndex));

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
		deferredFBO->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		deferredFBO->SetDrawAttachments(4, arr);
		deferredFBO->Clear(0.0f);

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
		deferredFBO->Unbind();

		{
			PR_PROFILE("Alpha Coverage pass");
			alphaCoveragePipeline->BindPipeline(nullptr);
			static_cast<GLAlphaCoverageShader*>(alphaCoveragePipeline->getShader())->UpdateUniforms(this, camera, lights);
			alphaCoveragePipeline->RenderPipeline();
			alphaCoveragePipeline->UnbindPipeline();
		}
		
		{
			PR_PROFILE("Deferred shading pass");
			deferredPipeline->BindPipeline(nullptr);
			static_cast<GLDeferredShader*>(deferredPipeline->getShader())->UpdateUniforms(this, camera, lights);
			deferredPipeline->RenderPipeline();
			deferredPipeline->UnbindPipeline();
		}

		{
			PR_PROFILE("FXAA pass");
			//fxaaPipeline->BindPipeline(nullptr);
			//static_cast<GLFXAAShader*>(fxaaPipeline->getShader())->UpdateUniforms(this, camera, lights);
			//fxaaPipeline->RenderPipeline();
			//fxaaPipeline->UnbindPipeline();
		}

		{
			PR_PROFILE("Show pass");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(outputImage, Vector3f(-1));
			renderPipeline->RenderPipeline();
			renderPipeline->UnbindPipeline();
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
};
