#include "Includes.hpp"
#include "VKRenderer.h"
#include "VKWaterSimulation.h"
#include "prehistoric/core/modules/water/Water.h"
#include "prehistoric/core/modules/water/WaterCapture.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"
#include "VKEnvironmentBackground.h"
#include "VKAtmosphereParameters.h"
#include "VKEnvironmentLighting.h"
#include "platform/vulkan/texture/VKTexture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"
#include "platform/vulkan/rendering/shaders/VkShader.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric {
    VKRenderer::VKRenderer(Window *window, Camera *camera, AssembledAssetManager *manager)
        : Renderer(window, camera, manager), commandPool(nullptr), renderpass(nullptr) {
        VKContext *context = (VKContext *) window->getContext();
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();

        VKDevice *device = (VKDevice *) context->getDevices();

        renderpass = std::make_unique<VKRenderpass>(device, swapchain);

        size_t NumImages = swapchain->getSwapchainImages().size();

        //Create command pool
        commandPool = std::make_unique<VKCommandPool>(device);

        for (size_t i = 0; i < NumImages; i++) {
            commandPool->AddCommandBuffer();
        }

        swapchain->setRenderpass(renderpass.get());
        //A nastly little cheat to avoid modifying everything in the Pipeline system because the Vulkan Graphics Pipelines require the renderpass
        background = std::make_unique<VKEnvironmentBackground>(window);
    }

    void VKRenderer::RequestEnvironmentBake(bool usePanorama) {
        panoramaOverride = usePanorama;
        bakeRequested = true;
    }

    Texture *VKRenderer::getOutputTexture() const {
        return renderpass->getEditorOutput();
    }

    void VKRenderer::UpdateEnvironmentLighting() {
        auto capture = EnvironmentCaptureSource::FromScene(
            !panoramaOverride && __AtmosphereConfig.scatteringEnabled ? atmosphere : nullptr, 0);
        if (!bakeRequested && !environmentCapture.NeedsBake(capture, __EnvironmentMapConfig.atmosphereRefreshSeconds))
            return;
        bool captureSky = capture.scattering;
        auto *device = static_cast<VKDevice *>(window->getContext()->getDevices());
        VkDescriptorImageInfo source{};
        std::unique_ptr<VKTexture> fallback;
        if (!captureSky) {
            source = background->LoadSource(window);
            if (!source.imageView) {
                fallback.reset(static_cast<VKTexture *>(TextureLoader::LoadTexture(window,
                    "engine://textures/default.png", Bilinear, ClampToEdge)));
                source = {
                    fallback->getSampler(), fallback->getImage()->getImageView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
            }
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);
        const auto limit = properties.limits.maxImageDimensionCube;
        const bool editor = Application::Get().isEditor();
        // The sample caps startup bake cost. The editor exposes these sizes to the user.
        uint32_t irradianceSize = std::clamp(__EnvironmentMapConfig.irradianceMapResolution, 1u, editor ? limit : 64u);
        uint32_t prefilterSize = std::clamp(__EnvironmentMapConfig.prefilterMapResolution, editor ? 1u : 2u,
                                            editor ? limit : 256u);
        uint32_t levels = editor
                              ? std::max(1u, __EnvironmentMapConfig.prefilterLevels)
                              : std::clamp(__EnvironmentMapConfig.prefilterLevels, 2u, 9u);
        const char *captureShader = captureSky ? "vulkan/atmosphere/capture_CS.spv" : "vulkan/ibl/convert_CS.spv";
        environmentLighting = std::make_unique<VKEnvironmentLighting>(
            device->getPhysicalDevice(), device->getDevice(), device->getGraphicsQueue().getQueue(),
            device->getQueueFamilyIndices().graphicsFamily, source, irradianceSize, prefilterSize, levels,
            std::clamp(__EnvironmentMapConfig.environmentMapResolution, 1u, limit),
            ResourceLoader::LoadShaderVK(captureShader),
            ResourceLoader::LoadShaderVK("vulkan/ibl/ibl_CS.spv"), VKEnvironmentLighting::DefaultLutSize,
            VKEnvironmentLighting::DefaultSamples, captureSky ? &capture.sky : nullptr, environmentLighting.get());
        environmentCapture.Complete(capture);
        bakeRequested = false;
        if (!captureSky && !fallback) background->SetPrefilter(environmentLighting->getPrefilter());
    }

    VKRenderer::~VKRenderer() {
        vkDeviceWaitIdle(((VKDevice *) window->getContext()->getDevices())->getDevice());
    }

    void VKRenderer::PrepareRendering() {
        VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
        frameReady = false;
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize((GLFWwindow *) window->getWindowHandle(), &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0)
            return;

        if (window->isResized()) {
            PR_PROFILE("Resizing window");

            VKContext *context = (VKContext *) window->getContext();
            VKDevice *device = (VKDevice *) context->getDevices();
            vkDeviceWaitIdle(device->getDevice());

            renderpass.reset();

            swapchain->SetWindowSize((uint32_t) framebufferWidth, (uint32_t) framebufferHeight);
            VkExtent2D extent = swapchain->getSwapchainExtent();
            uint32_t width = extent.width;
            uint32_t height = extent.height;
            camera->SetProjection(camera->getFov(), (float) width, (float) height);

            renderpass = std::make_unique<VKRenderpass>(device, swapchain);
            swapchain->setRenderpass(renderpass.get());

            size_t NumImages = swapchain->getSwapchainImages().size();

            //Create command pool
            commandPool->DeleteCommandBuffers();
            for (size_t i = 0; i < NumImages; i++) {
                commandPool->AddCommandBuffer();
            }

            ResizePipelines(width, height);

            background->RecreatePipeline();

            window->setResized(false);
        }

        if (!swapchain->GetNextImageIndex())
            return;
        frameReady = true;
    }

    void VKRenderer::EndRendering() {
        if (frameReady) {
            PR_PROFILE("Submitting");
            window->Render(getDrawCommandBuffer());
        }

        waters.clear();
        ClearFrameSubmissions();
    }

    void VKRenderer::RenderScene() {
        renderpass->RetainWaterTargets(waters);
        for (auto* water : waters)
        {
            renderpass->GetWaterTargets(water);
            water->captureDepthScale = MakeWaterCapture(camera->getViewMatrix(),camera->getProjectionMatrix(),
                camera->getPosition(),water->height(),false,true).depthScale;
        }
        ProcessDraws(nullptr);

        auto *swapchain = static_cast<VKSwapchain *>(window->getSwapchain());
        uint32_t index = swapchain->getAquiredImageIndex();
        VKCommandBuffer *buffer = commandPool->getCommandBuffer(index);
        buffer->BindBuffer();
        for (auto* water : waters)
        {
            water->simulation().Record(buffer->getCommandBuffer(), float(Time::getTimeFromStartNanoseconds()*1.0e-9));
            auto& targets = renderpass->GetWaterTargets(water);
            for (bool reflected : {true,false})
            {
                const Matrix4f view = camera->getViewMatrix();
                const Matrix4f projection = camera->getProjectionMatrix();
                const auto pass = MakeWaterCapture(view,projection,camera->getPosition(),water->height(),reflected,true);
                const auto target = reflected ? targets.reflectionFrame.get() : targets.refractionFrame.get();
                renderpass->BeginRenderpass(buffer, Vector4f(0,0,0,1), target->getFramebuffer());
                if (pass.sky)
                {
                    camera->setViewMatrix(pass.view);
                    camera->setProjectionMatrix(pass.projection);
                    background->Draw(buffer->getCommandBuffer(),camera,__AtmosphereConfig.scatteringEnabled ? atmosphere : nullptr,environmentPreviewLod);
                    camera->setViewMatrix(view);
                    camera->setProjectionMatrix(projection);
                }
                if (pass.visible)
                {
                    CaptureConstants constants{pass.clippedProjection * pass.view, Vector4f(pass.eye,1)};
                    ProcessDraws(buffer,&constants);
                }
                vkCmdEndRenderPass(buffer->getCommandBuffer());
            }
        }
        renderpass->BeginRenderpass(buffer, swapchain->getClearColour());
        background->Draw(buffer->getCommandBuffer(), camera,
                         __AtmosphereConfig.scatteringEnabled ? atmosphere : nullptr, environmentPreviewLod);
        ProcessDraws(buffer);
    }

    void VKRenderer::RenderHDR() {
        const bool underwater = __WaterConfig.underwaterPostProcessing && std::any_of(waters.begin(),waters.end(),
            [](const Water* water) { return water->isCameraUnderwater(); });
        renderpass->RenderHDR(getDrawCommandBuffer(), bloomEnabled, underwater);
    }

    void VKRenderer::FinishFrame() {
        VKCommandBuffer *buffer = getDrawCommandBuffer();
        if (auto *ui = Application::Get().getImGuiLayer()) ui->Draw();
        vkCmdEndRenderPass(buffer->getCommandBuffer());
        buffer->UnbindBuffer();
    }

    void VKRenderer::ProcessDraws(VKCommandBuffer *buffer, const CaptureConstants* capture) {
        // Each shader owns its descriptor pool. Both traversals assign the same slots.
        struct Slots {
            uint32_t material = 0;
            uint32_t object = 0;
        };
        std::unordered_map<VKShader *, Slots> shaderSlots;
        Slots *slots = nullptr;
        VKShader *shader = nullptr;
        auto begin = [&](Pipeline *pipeline, bool wireframe) {
            shader = static_cast<VKShader *>(pipeline->getShader());
            slots = &shaderSlots[shader];
            if (buffer) static_cast<VKGraphicsPipeline *>(pipeline)->BindPipeline(buffer, wireframe);
        };
        auto beginScene = [&](Pipeline *pipeline) {
            begin(pipeline, isWireframeMode());
            if (buffer)
            {
                CaptureConstants mainView{Matrix4f::Identity(), Vector4f(0,0,0,0)};
                vkCmdPushConstants(buffer->getCommandBuffer(), shader->GetPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS,
                    0, sizeof(mainView), capture ? capture : &mainView);
                shader->BindGlobalSets();
            }
            else shader->UpdateGlobalUniforms(this);
        };
        auto material = [&](Pipeline *, Material *material) {
            if (buffer) shader->BindTextureSets(slots->material++);
            else shader->UpdateMaterialUniforms(material, slots->material++);
        };
        auto object = [&](Pipeline *pipeline, RenderableComponent *renderable) {
            if (buffer) {
                auto slot = slots->object++;
                if (capture && !renderable->hasReflection()) return;
                shader->BindObjectSets(slot);
                pipeline->RenderPipeline();
            } else {
                shader->UpdateObjectUniforms(renderable->getParent(), slots->object++);
            }
        };
        auto end = [&](Pipeline *pipeline) {
            if (buffer) pipeline->UnbindPipeline();
        };
        VisitBatches(models_3d, beginScene, material, object, end);
        if (capture) return;
        VisitBatches(models_transparency, beginScene, material, object, end);
        for (const auto &[pipeline, objects]: models_2d) {
            begin(pipeline, false);
            for (RenderableComponent *renderable: objects) {
                object(pipeline, renderable);
            }
            end(pipeline);
        }
    }
}