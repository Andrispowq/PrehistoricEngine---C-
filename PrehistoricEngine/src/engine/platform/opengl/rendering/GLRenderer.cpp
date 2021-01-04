#include "Includes.hpp"
#include "GLRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

#include "platform/opengl/rendering/shaders/deferred/GLDeferredShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLAlphaCoverageShader.h"
#include "platform/opengl/rendering/shaders/deferred/GLFXAAShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"

namespace Prehistoric
{
	GLRenderer::GLRenderer(Window* window, Camera* camera, ResourceStorage* resourceStorage)
		: Renderer(window, camera, resourceStorage), deferredFBO{nullptr}
	{
		deferredFBO = std::make_unique<GLFramebuffer>(window);

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		positionMetalic = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
		albedoRoughness = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
		normalLit = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
		emissionExtra = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);

		alphaCoverage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
		outputImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
		//fxaaTexture = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);

		deferredFBO->Bind();
		deferredFBO->addDepthAttachment(width, height, true);
		
		deferredFBO->addColourAttachment2D(positionMetalic, 0);
		deferredFBO->addColourAttachment2D(albedoRoughness, 1);
		deferredFBO->addColourAttachment2D(normalLit, 2);
		deferredFBO->addColourAttachment2D(emissionExtra, 3);

		deferredFBO->Check();
		deferredFBO->Unbind();

		quadVBO = resourceStorage->storeVertexBuffer(ModelFabricator::CreateQuad(window));
		quadVBO.pointer->setFrontFace(FrontFace::DOUBLE_SIDED);

		alphaCoverageShader = resourceStorage->loadShader("alpha_coverage").value();
		deferredShader = resourceStorage->loadShader("deferred").value();
		fxaaShader = resourceStorage->loadShader("fxaa").value();
		renderShader = resourceStorage->loadShader("gui").value();

		alphaCoveragePipeline = new GLComputePipeline(window, resourceStorage, alphaCoverageShader);
		deferredPipeline = new GLComputePipeline(window, resourceStorage, deferredShader);
		fxaaPipeline = new GLComputePipeline(window, resourceStorage, fxaaShader);
		renderPipeline = new GLGraphicsPipeline(window, resourceStorage, renderShader, quadVBO);

		static_cast<GLComputePipeline*>(alphaCoveragePipeline)->setInvocationSize({ width / 16, height / 16, 1 });
		static_cast<GLComputePipeline*>(alphaCoveragePipeline)->addTextureBinding(0, alphaCoverage, WRITE_ONLY);

		static_cast<GLComputePipeline*>(deferredPipeline)->setInvocationSize({ width / 16, height / 16, 1 });
		static_cast<GLComputePipeline*>(deferredPipeline)->addTextureBinding(0, outputImage, WRITE_ONLY);

		//static_cast<GLComputePipeline*>(fxaaPipeline)->setInvocationSize({ width / 16, height / 16, 1 });
		//static_cast<GLComputePipeline*>(fxaaPipeline)->addTextureBinding(0, fxaaTexture, WRITE_ONLY);
	}

	GLRenderer::~GLRenderer()
	{
		delete positionMetalic;
		delete albedoRoughness;
		delete normalLit;
		delete emissionExtra;
		
		delete alphaCoverage;
		delete outputImage;
		//delete fxaaTexture;

		delete deferredPipeline;
		//delete fxaaPipeline;
		delete renderPipeline;
	}

	void GLRenderer::PrepareRendering()
	{
		if (window->isResized())
		{
			PR_PROFILE("Window resize pass");

			uint32_t width = window->getWidth();
			uint32_t height = window->getHeight();

			window->getSwapchain()->SetWindowSize(width, height);

			//Recreate the FBO and the images
			delete positionMetalic;
			delete albedoRoughness;
			delete normalLit;
			delete emissionExtra;

			delete alphaCoverage;
			delete outputImage;
			//delete fxaaTexture;

			positionMetalic = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
			albedoRoughness = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
			normalLit = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);
			emissionExtra = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false, true);

			static_cast<GLComputePipeline*>(alphaCoveragePipeline)->removeTextureBinding(0);
			alphaCoverage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
			static_cast<GLComputePipeline*>(alphaCoveragePipeline)->setInvocationSize({ width / 16, height / 16, 1 });
			static_cast<GLComputePipeline*>(alphaCoveragePipeline)->addTextureBinding(0, alphaCoverage, WRITE_ONLY);

			static_cast<GLComputePipeline*>(deferredPipeline)->removeTextureBinding(0);
			outputImage = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
			static_cast<GLComputePipeline*>(deferredPipeline)->setInvocationSize({ width / 16, height / 16, 1 });
			static_cast<GLComputePipeline*>(deferredPipeline)->addTextureBinding(0, outputImage, WRITE_ONLY);

			//static_cast<GLComputePipeline*>(fxaaPipeline)->removeTextureBinding(0);
			//fxaaTexture = GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false);
			//static_cast<GLComputePipeline*>(fxaaPipeline)->setInvocationSize({ width / 16, height / 16, 1 });
			//static_cast<GLComputePipeline*>(fxaaPipeline)->addTextureBinding(0, fxaaTexture, WRITE_ONLY);

			deferredFBO->Bind();
			deferredFBO->addDepthAttachment(width, height, true);

			deferredFBO->addColourAttachment2D(positionMetalic, 0);
			deferredFBO->addColourAttachment2D(albedoRoughness, 1);
			deferredFBO->addColourAttachment2D(normalLit, 2);
			deferredFBO->addColourAttachment2D(emissionExtra, 3);

			deferredFBO->Check();
			deferredFBO->Unbind();

			//Recreate the pipelines
			std::vector<Pipeline*> pipes = resourceStorage->get<Pipeline>();
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

		//Clear the lists for the next iteration
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
			if (FrameworkConfig::api == OpenGL)
				EnvironmentMapRenderer::instance->RenderCube(camera);
		}

		{
			PR_PROFILE("Main pass");
			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateShaderUniforms(camera, lights);
				pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

				for (auto renderer : pipeline.second)
				{
					renderer->BatchRender();
				}

				pl->UnbindPipeline();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateShaderUniforms(camera, lights);
				pl->getShader()->UpdateSharedUniforms(pipeline.second[0]->getParent()); //Safe -> there is at least 1 element in the array

				for (auto renderer : pipeline.second)
				{
					renderer->BatchRender();
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

		//fxaaPipeline->BindPipeline(nullptr);
		//static_cast<GLFXAAShader*>(fxaaPipeline->getShader())->UpdateUniforms(this, camera, lights);
		//fxaaPipeline->RenderPipeline();
		//fxaaPipeline->UnbindPipeline();

		{
			PR_PROFILE("Show pass");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(outputImage);//fxaaTexture);
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
