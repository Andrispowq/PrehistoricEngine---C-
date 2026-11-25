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

namespace Prehistoric {
    GLRenderer::GLRenderer(Window *window, Camera *camera, AssembledAssetManager *manager)
        : Renderer(window, camera, manager), atmospherePass(*manager) {
        depthPass = std::make_unique<GLDepthPass>(this);
        shadowDepthPass = std::make_unique<GLShadowDepthPass>(this);
        lightCullingPass = std::make_unique<GLLightCullingPass>(this);
        mainPass = std::make_unique<GLMainPass>(this);
        bloomPass = std::make_unique<GLBloomPass>(this);
        volumetricPostProcessingPass = std::make_unique<GLVolumetricPostProcessingPass>(this);
        hdrPass = std::make_unique<GLHDRPass>(this);

        voxelPass = std::make_unique<GLVoxelRendererPass>(this);

        rtxPass = std::make_unique<GLRayTracingPass>(this);

        environmentLighting = std::make_unique<EnvironmentMapRenderer>(window, manager, atmospherePass);

        AssetManager *man = manager->getAssetManager();

        renderShader = man->loadShader(ShaderName::Gui).value();

        quad = man->storeVertexBuffer(ModelFabricator::CreateQuad(window));
        quad->setFrontFace(FrontFace::DOUBLE_SIDED);

        renderPipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, renderShader, quad));

        manager->addReference<Pipeline>(renderPipeline.handle);
    }

    GLRenderer::~GLRenderer() {
        manager->removeReference<Pipeline>(renderPipeline.handle);
    }

    void GLRenderer::PrepareRendering() {
        if (window->isResized()) {
            PR_PROFILE("Window resize pass");
            uint32_t width = window->getWidth();
            uint32_t height = window->getHeight();

            depthPass->OnResized();
            lightCullingPass->OnResized();
            mainPass->OnResized();
            bloomPass->OnResized();
            volumetricPostProcessingPass->OnResized();
            hdrPass->OnResized();

            rtxPass->OnResized();

            window->getSwapchain()->SetWindowSize(width, height);

            ResizePipelines(width, height);

            window->setResized(false);
        }
    }

    void GLRenderer::EndRendering() {
        window->Render(nullptr);

        ClearFrameSubmissions();
        waters.clear();
    }

    void GLRenderer::DrawModelBatches(const ModelBatches &batches) {
        VisitBatches(batches, [this](Pipeline *pipeline) {
                         pipeline->BindPipeline(nullptr);
                         pipeline->getShader()->UpdateGlobalUniforms(this);
                         lightCullingPass->getLightBuffer()->BindBase(nullptr, 0);
                         lightCullingPass->getVisibleLightIndicesBuffer()->BindBase(nullptr, 1);
                     }, [](Pipeline *pipeline, Material *material) {
                         pipeline->getShader()->UpdateMaterialUniforms(material, 0);
                     }, [](Pipeline *, RenderableComponent *object) {
                         object->BatchRender();
                     }, [this](Pipeline *pipeline) {
                         lightCullingPass->getVisibleLightIndicesBuffer()->UnbindBase(1);
                         lightCullingPass->getLightBuffer()->UnbindBase(0);
                         pipeline->UnbindPipeline();
                     });
    }

    void GLRenderer::RequestEnvironmentBake(bool usePanorama) {
        environmentLighting->RequestBake(usePanorama);
    }

    void GLRenderer::DrawBackground(Camera *camera) {
        if (atmosphere) atmospherePass.DrawScene(*atmosphere, *camera);
        else if (environmentLighting->enabled) environmentLighting->RenderCube(camera, environmentPreviewLod);
    }

    void GLRenderer::UpdateEnvironmentLighting() {
        // The directional light already supplies direct sun with scene shadows.
        const auto *sun = ShadowCascades::FindSun(*this);
        environmentLighting->UpdateEnvironment(atmosphere, !atmosphere || sun != atmosphere->getSun(),
                                               sceneEnvironmentMap);
    }

    void GLRenderer::RenderScene() {
        const auto &voxelSettings = voxelPass->GetSettings();
        voxelPass->Render();
        if (voxelSettings.debugView && voxelPass->IsReady()) return;

        depthPass->Render();

        //shadow pass
        if (!voxelPass->UsesVoxelSunShadows()) {
            renderingShadow = true;
            shadowDepthPass->Render();
            renderingShadow = false;
        }

        lightCullingPass->Render();
        for (Water *water: waters) {
            water->RenderCaptures(this);
        }

        mainPass->Render();
    }

    bool GLRenderer::isUnderwater() const {
        for (const Water *water: waters) {
            if (water->isCameraUnderwater()) return true;
        }
        return false;
    }

    void GLRenderer::RenderHDR() {
        if (voxelPass->IsReady()) {
            const auto &settings = voxelPass->GetSettings();
            if (settings.debugView) {
                voxelPass->Composite(getOutputTexture());
                return;
            }
            if (settings.globalIllumination)
                voxelPass->Composite(mainPass->getColourImage().pointer);
        }
        if (bloomEnabled) {
            bloomPass->Render();
        }

        if (!getVolumetricSpheres().empty()) {
            volumetricPostProcessingPass->Render();
        }
        hdrPass->Render();
    }

    void GLRenderer::FinishFrame() {
        {
            Texture *result = hdrPass->getOutputImage().pointer;

            PR_PROFILE("Render to screen");
            renderPipeline->BindPipeline(nullptr);
            static_cast<GLGUIShader *>(renderPipeline->getShader())->UpdateCustomUniforms(result, -1);
            renderPipeline->RenderPipeline();
            renderPipeline->UnbindPipeline();
        }

        //TODO: disable alpha blending and depth testing
        {
            PR_PROFILE("GUI pass");
            for (const auto &pipeline: models_2d) {
                Pipeline *pl = pipeline.first;

                pl->BindPipeline(nullptr);

                for (auto renderer: pipeline.second) {
                    renderer->BatchRender();
                }

                pl->UnbindPipeline();
            }
        }
    }
};