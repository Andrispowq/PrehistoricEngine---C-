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
	extern bool bloomEnabled; //TODO: this is a shitty way to achieve this

	GLRenderer::GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: Renderer(window, camera, manager)
	{
		depthPass = new GLDepthPass(this);
		shadowDepthPass = new GLShadowDepthPass(this);
		lightCullingPass = new GLLightCullingPass(this);
		mainPass = new GLMainPass(this);
		bloomPass = new GLBloomPass(this);
		volumetricPostProcessingPass = new GLVolumetricPostProcessingPass(this);
		hdrPass = new GLHDRPass(this);

		rtxPass = new GLRayTracingPass(this);

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();

		renderShader = man->loadShader(ShaderName::Gui).value();

		quad = man->storeVertexBuffer(ModelFabricator::CreateQuad(window));
		quad->setFrontFace(FrontFace::DOUBLE_SIDED);

		renderPipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, renderShader, quad));

		manager->addReference<Pipeline>(renderPipeline.handle);
	}

	GLRenderer::~GLRenderer()
	{
		delete depthPass;
		delete shadowDepthPass;
		delete lightCullingPass;
		delete mainPass;
		delete bloomPass;
		delete volumetricPostProcessingPass;
		delete hdrPass;

		delete rtxPass;

		AssetManager* man = manager->getAssetManager();
		manager->removeReference<Pipeline>(renderPipeline.handle);
	}

	void GLRenderer::PrepareRendering()
	{
		if (window->isResized())
		{
			PR_PROFILE("Window resize pass");
			uint32_t width = window->getWidth();
			uint32_t height = window->getHeight();

			depthPass->OnResized();
			shadowDepthPass->OnResized();
			lightCullingPass->OnResized();
			mainPass->OnResized();
			bloomPass->OnResized();
			volumetricPostProcessingPass->OnResized();
			hdrPass->OnResized();

			rtxPass->OnResized();

			window->getSwapchain()->SetWindowSize(width, height);

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
		models_water.clear();
		lights.clear();
	}

	void GLRenderer::Render()
	{ 
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		depthPass->Render();
		
		//shadow pass
		renderingShadow = true;
		//shadowDepthPass->Render();
		renderingShadow = false;

		lightCullingPass->Render();
		mainPass->Render();

		//rtxPass->Render();

		if (bloomEnabled)
		{
			bloomPass->Render();
		}

		volumetricPostProcessingPass->Render();
		hdrPass->Render();

		{
			PR_PROFILE("Render to screen");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(hdrPass->getOutputImage().pointer, -1);
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
};
