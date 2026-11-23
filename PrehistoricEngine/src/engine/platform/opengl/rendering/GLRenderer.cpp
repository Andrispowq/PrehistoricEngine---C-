#include "Includes.hpp"
#include "GLRenderer.h"
#include "prehistoric/core/modules/water/Water.h"

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
		: Renderer(window, camera, manager), atmospherePass(*manager)
	{
		depthPass = new GLDepthPass(this);
		shadowDepthPass = new GLShadowDepthPass(this);
		lightCullingPass = new GLLightCullingPass(this);
		mainPass = new GLMainPass(this);
		bloomPass = new GLBloomPass(this);
		volumetricPostProcessingPass = new GLVolumetricPostProcessingPass(this);
		hdrPass = new GLHDRPass(this);

		voxelPass = new GLVoxelRendererPass(this);

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

		delete voxelPass;

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

			voxelPass->OnResized();

			rtxPass->OnResized();

			window->getSwapchain()->SetWindowSize(width, height);

			ResizePipelines(width, height);

			window->setResized(false);
		}
	}

	void GLRenderer::EndRendering()
	{
		window->Render(nullptr);

		ClearFrameSubmissions();
		waters.clear();
	}

    void GLRenderer::DrawModelBatches(const ModelBatches& batches)
    {
        VisitBatches(batches, [this](Pipeline* pipeline)
        {
            pipeline->BindPipeline(nullptr);
            pipeline->getShader()->UpdateGlobalUniforms(this);
            lightCullingPass->getLightBuffer()->BindBase(nullptr, 0);
            lightCullingPass->getVisibleLightIndicesBuffer()->BindBase(nullptr, 1);
        }, [](Pipeline* pipeline, Material* material)
        {
            pipeline->getShader()->UpdateMaterialUniforms(material, 0);
        }, [](Pipeline*, RenderableComponent* object)
        {
            object->BatchRender();
        }, [this](Pipeline* pipeline)
        {
            lightCullingPass->getVisibleLightIndicesBuffer()->UnbindBase(1);
            lightCullingPass->getLightBuffer()->UnbindBase(0);
            pipeline->UnbindPipeline();
        });
    }

	void GLRenderer::Render()
	{ 
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

        if (EnvironmentMapRenderer::instance) EnvironmentMapRenderer::instance->UpdateEnvironment(atmosphere);
        for (Water* water : waters)
        {
            water->RenderCaptures(this);
        }

		depthPass->Render();
		
		//shadow pass
		renderingShadow = true;
		shadowDepthPass->Render();
		renderingShadow = false;

		lightCullingPass->Render();
		mainPass->Render();

		//voxelPass->Render();

		//rtxPass->Render();

		if (bloomEnabled)
		{
			bloomPass->Render();
		}

		volumetricPostProcessingPass->Render();
		hdrPass->Render();

		{
			Texture* result = hdrPass->getOutputImage().pointer;
			if (wireframeMode)
			{
				//result = voxelPass->getOutputImage().pointer;
			}

			PR_PROFILE("Render to screen");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(result, -1);
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
