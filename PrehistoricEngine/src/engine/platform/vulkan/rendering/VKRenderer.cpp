#include "Includes.hpp"
#include "VKRenderer.h"
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
        primaryFramebuffers.resize(NumImages);

        for (size_t i = 0; i < NumImages; i++) {
            primaryFramebuffers[i] = renderpass->CreateFramebuffer(static_cast<uint32_t>(i));
        }

        //Create command pool
        commandPool = std::make_unique<VKCommandPool>(device);

        for (size_t i = 0; i < NumImages; i++) {
            commandPool->AddCommandBuffer();
        }

        swapchain->setRenderpass(renderpass.get());
        //A nastly little cheat to avoid modifying everything in the Pipeline system because the Vulkan Graphics Pipelines require the renderpass
        background = std::make_unique<VKEnvironmentBackground>(window);
    }

    void VKRenderer::UpdateEnvironmentLighting()
    {
        auto capture = EnvironmentCaptureSource::FromScene(__AtmosphereConfig.scatteringEnabled ? atmosphere : nullptr, 0);
        if (!environmentCapture.NeedsBake(capture, __EnvironmentMapConfig.atmosphereRefreshSeconds)) return;
        bool captureSky = capture.scattering;
        auto* device = static_cast<VKDevice*>(window->getContext()->getDevices());
        VkDescriptorImageInfo source{};
        std::unique_ptr<VKTexture> fallback;
        if (!captureSky)
        {
            source = background->LoadSource(window);
            if (!source.imageView)
            {
                fallback.reset(static_cast<VKTexture*>(TextureLoader::LoadTexture(window,
                    "engine://textures/default.png", Bilinear, ClampToEdge)));
                source = {fallback->getSampler(), fallback->getImage()->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            }
        }

        // Cap startup work independently of the more expensive OpenGL bake settings.
        // Maps are immutable and survive swapchain recreation.
        uint32_t irradianceSize = std::clamp(uint32_t(__EnvironmentMapConfig.irradianceMapResolution), 1u, 64u);
        uint32_t prefilterSize = std::clamp(uint32_t(__EnvironmentMapConfig.prefilterMapResolution), 2u, 256u);
        uint32_t levels = std::clamp(uint32_t(__EnvironmentMapConfig.prefilterLevels), 2u, 9u);
        const char* captureShader = captureSky ? "vulkan/atmosphere/capture_CS.spv" : "vulkan/ibl/convert_CS.spv";
        environmentLighting = std::make_unique<VKEnvironmentLighting>(
            device->getPhysicalDevice(), device->getDevice(), device->getGraphicsQueue().getQueue(),
            device->getQueueFamilyIndices().graphicsFamily, source, irradianceSize, prefilterSize, levels,
            __EnvironmentMapConfig.environmentMapResolution, ResourceLoader::LoadShaderVK(captureShader),
            ResourceLoader::LoadShaderVK("vulkan/ibl/ibl_CS.spv"), VKEnvironmentLighting::DefaultLutSize,
            VKEnvironmentLighting::DefaultSamples, captureSky ? &capture.sky : nullptr, environmentLighting.get());
        environmentCapture.Complete(capture);
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

            primaryFramebuffers.clear();
            delete renderpass.release();

            swapchain->SetWindowSize((uint32_t) framebufferWidth, (uint32_t) framebufferHeight);
            VkExtent2D extent = swapchain->getSwapchainExtent();
            uint32_t width = extent.width;
            uint32_t height = extent.height;
            camera->SetProjection(camera->getFov(), (float) width, (float) height);

            renderpass = std::make_unique<VKRenderpass>(device, swapchain);
            swapchain->setRenderpass(renderpass.get());

            size_t NumImages = swapchain->getSwapchainImages().size();
            primaryFramebuffers.resize(NumImages);
            for (size_t i = 0; i < NumImages; i++) {
                primaryFramebuffers[i] = renderpass->CreateFramebuffer(static_cast<uint32_t>(i));
            }

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

        /*uint32_t index = swapchain->getAquiredImageIndex();
		VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);

		buffer->BindBuffer();
		renderpass->BeginRenderpass(buffer, primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(), swapchain->getClearColour());*/
    }

    void VKRenderer::EndRendering() {
        if (frameReady) {
            VKSwapchain *swapchain = (VKSwapchain *) window->getSwapchain();
            uint32_t index = swapchain->getAquiredImageIndex();
            VKCommandBuffer *buffer = commandPool->getCommandBuffer(index);

            /*renderpass->EndRenderpass(buffer);
			buffer->UnbindBuffer();*/

            {
                PR_PROFILE("Submitting");
                window->Render(buffer);
            }
        }

        ClearFrameSubmissions();
    }

    void VKRenderer::Render() {
        if (!frameReady)
            return;

        UpdateEnvironmentLighting();

        PR_PROFILE("Update uniforms");

        ProcessDraws(nullptr);

        auto* swapchain = static_cast<VKSwapchain*>(window->getSwapchain());
        uint32_t index = swapchain->getAquiredImageIndex();
        VKCommandBuffer* buffer = commandPool->getCommandBuffer(index);
        buffer->BindBuffer();
        renderpass->BeginRenderpass(buffer, primaryFramebuffers[index].get(), swapchain->getSwapchainExtent(),
                                    swapchain->getClearColour());
        background->Draw(buffer->getCommandBuffer(), camera, __AtmosphereConfig.scatteringEnabled ? atmosphere : nullptr);
        ProcessDraws(buffer);
        renderpass->EndRenderpass(buffer);
        buffer->UnbindBuffer();
    }

    void VKRenderer::ProcessDraws(VKCommandBuffer* buffer)
    {
        // Each shader owns its descriptor pool. Both traversals assign the same slots.
        struct Slots
        {
            uint32_t material = 0;
            uint32_t object = 0;
        };
        std::unordered_map<VKShader*, Slots> shaderSlots;
        Slots* slots = nullptr;
        VKShader* shader = nullptr;
        auto begin = [&](Pipeline* pipeline)
        {
            shader = static_cast<VKShader*>(pipeline->getShader());
            slots = &shaderSlots[shader];
            if (buffer) pipeline->BindPipeline(buffer);
        };
        auto beginScene = [&](Pipeline* pipeline)
        {
            begin(pipeline);
            if (buffer) shader->BindGlobalSets();
            else shader->UpdateGlobalUniforms(this);
        };
        auto material = [&](Pipeline*, Material* material)
        {
            if (buffer) shader->BindTextureSets(slots->material++);
            else shader->UpdateMaterialUniforms(material, slots->material++);
        };
        auto object = [&](Pipeline* pipeline, RenderableComponent* renderable)
        {
            if (buffer)
            {
                shader->BindObjectSets(slots->object++);
                pipeline->RenderPipeline();
            }
            else
            {
                shader->UpdateObjectUniforms(renderable->getParent(), slots->object++);
            }
        };
        auto end = [&](Pipeline* pipeline)
        {
            if (buffer) pipeline->UnbindPipeline();
        };
        VisitBatches(models_3d, beginScene, material, object, end);
        VisitBatches(models_transparency, beginScene, material, object, end);
        for (const auto& [pipeline, objects] : models_2d)
        {
            begin(pipeline);
            for (RenderableComponent* renderable : objects)
            {
                object(pipeline, renderable);
            }
            end(pipeline);
        }
    }
}
