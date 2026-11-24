#include "Includes.hpp"
#include "VKImGui.h"

#include "platform/vulkan/framework/context/VKContext.h"
#include "backends/imgui_impl_vulkan.cpp"

namespace Prehistoric {
    static constexpr uint32_t TexturesPerPool = 64;

    static void CheckImGuiResult(VkResult result) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) throw result;
        if (result < 0) throw std::runtime_error("Vulkan ImGui operation failed (" + std::to_string(result) + ")");
    }

    VkDescriptorPool VKImGui::CreatePool(uint32_t capacity) {
        VkDescriptorPoolSize size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, capacity};
        VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        info.maxSets = capacity;
        info.poolSizeCount = 1;
        info.pPoolSizes = &size;
        VkDescriptorPool pool{};
        CheckImGuiResult(vkCreateDescriptorPool(device->getDevice(), &info, nullptr, &pool));
        return pool;
    }

    VKImGui::VKImGui(VKContext *context, VKSwapchain *swapchain)
        : context(context), device(static_cast<VKDevice *>(context->getDevices())), swapchain(swapchain) {
        InitRenderer();
    }

    void VKImGui::InitRenderer() {
        format = swapchain->getSwapchainImageFormat();
        imageCount = static_cast<uint32_t>(swapchain->getSwapchainImages().size());
        fontPool = CreatePool(1);
        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = context->getInstance()->getInstance();
        info.PhysicalDevice = device->getPhysicalDevice();
        info.Device = device->getDevice();
        info.QueueFamily = device->getQueueFamilyIndices().graphicsFamily;
        info.Queue = device->getGraphicsQueue().getQueue();
        info.MinImageCount = 2;
        info.ImageCount = imageCount;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.DescriptorPool = fontPool;
        info.CheckVkResultFn = CheckImGuiResult;
        ImGui_ImplVulkan_Init(&info, swapchain->getRenderpass()->getPresentPass());

        auto &platform = ImGui::GetPlatformIO();
        platform.Renderer_RenderWindow = DrawWindow;
        platform.Renderer_SwapBuffers = PresentWindow;

        VKCommandPool upload(device);
        upload.AddCommandBuffer();
        auto *buffer = upload.getCommandBuffer(0);
        buffer->BindBuffer();
        ImGui_ImplVulkan_CreateFontsTexture(buffer->getCommandBuffer());
        buffer->UnbindBuffer();
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &buffer->getCommandBuffer();
        CheckImGuiResult(vkQueueSubmit(info.Queue, 1, &submit, VK_NULL_HANDLE));
        CheckImGuiResult(vkQueueWaitIdle(info.Queue));
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void VKImGui::ShutdownRenderer() {
        CheckImGuiResult(vkDeviceWaitIdle(device->getDevice()));
        for (const auto &entry: platformPipelines) vkDestroyPipeline(device->getDevice(), entry.second, nullptr);
        platformPipelines.clear();
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(device->getDevice(), fontPool, nullptr);
    }

    VKImGui::~VKImGui() {
        ShutdownRenderer();
        for (auto pool: texturePools) vkDestroyDescriptorPool(device->getDevice(), pool, nullptr);
    }

    void VKImGui::Begin() {
        // Swapchain recreation has already waited for the device. Keep the UI context and platform windows.
        auto *backend = ImGui_ImplVulkan_GetBackendData();
        backend->RenderPass = swapchain->getRenderpass()->getPresentPass();
        if (format != swapchain->getSwapchainImageFormat()) {
            format = swapchain->getSwapchainImageFormat();
            vkDestroyPipeline(device->getDevice(), backend->Pipeline, nullptr);
            ImGui_ImplVulkan_CreatePipeline(device->getDevice(), nullptr, VK_NULL_HANDLE, backend->RenderPass,
                                            VK_SAMPLE_COUNT_1_BIT, &backend->Pipeline, 0);
        }
        if (imageCount != swapchain->getSwapchainImages().size()) {
            imageCount = static_cast<uint32_t>(swapchain->getSwapchainImages().size());
            ImGui_ImplVulkanH_DestroyAllViewportsRenderBuffers(device->getDevice(), nullptr);
            backend->VulkanInitInfo.ImageCount = imageCount;
        }
        // SwapBuffers waits for main presentation. Detached draws finish below before pool reuse.
        for (auto pool: texturePools) CheckImGuiResult(vkResetDescriptorPool(device->getDevice(), pool, 0));
        textureCount = 0;
        ImGui_ImplVulkan_NewFrame();
    }

    void VKImGui::Draw(VkCommandBuffer command) {
        auto *data = ImGui::GetDrawData();
        auto extent = swapchain->getSwapchainExtent();
        if (data->DisplaySize.x <= 0 || data->DisplaySize.y <= 0) return;
        data->FramebufferScale = ImVec2(extent.width / data->DisplaySize.x, extent.height / data->DisplaySize.y);
        ImGui_ImplVulkan_RenderDrawData(data, command);
    }

    VkDescriptorSet VKImGui::BindTexture(VKTexture *texture) {
        const size_t pool = textureCount++ / TexturesPerPool;
        if (pool == texturePools.size()) texturePools.push_back(CreatePool(TexturesPerPool));
        // This backend version allocates user sets from its configured pool.
        auto &info = ImGui_ImplVulkan_GetBackendData()->VulkanInitInfo;
        info.DescriptorPool = texturePools[pool];
        auto set = ImGui_ImplVulkan_AddTexture(texture->getSampler(), texture->getImage()->getImageView(),
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        info.DescriptorPool = fontPool;
        return set;
    }

    void VKImGui::DrawWindow(ImGuiViewport *viewport, void *renderer) {
        auto &ui = *static_cast<VKImGui *>(renderer);
        auto &window = static_cast<ImGui_ImplVulkan_ViewportData *>(viewport->RendererUserData)->Window;
        int width, height;
        glfwGetFramebufferSize(static_cast<GLFWwindow *>(viewport->PlatformHandle), &width, &height);
        if (width <= 0 || height <= 0) {
            ui.skippedWindows.insert(viewport->ID);
            return;
        }
        if (width != window.Width || height != window.Height)
            ImGui_ImplVulkan_SetWindowSize(viewport, ImVec2(float(width), float(height)));
        viewport->DrawData->FramebufferScale =
                ImVec2(window.Width / viewport->Size.x, window.Height / viewport->Size.y);

        // Platform render passes can have a different attachment format and dependencies.
        auto &pipeline = ui.platformPipelines[window.SurfaceFormat.format];
        if (!pipeline)
            ImGui_ImplVulkan_CreatePipeline(ui.device->getDevice(), nullptr, VK_NULL_HANDLE, window.RenderPass,
                                            VK_SAMPLE_COUNT_1_BIT, &pipeline, 0);
        window.Pipeline = pipeline;
        try {
            ImGui_ImplVulkan_RenderWindow(viewport, nullptr);
        } catch (VkResult) {
            window.Pipeline = VK_NULL_HANDLE;
            ImGui_ImplVulkan_SetWindowSize(viewport, ImVec2(float(width), float(height)));
            ui.skippedWindows.insert(viewport->ID);
            return;
        }
        window.Pipeline = VK_NULL_HANDLE;
    }

    void VKImGui::PresentWindow(ImGuiViewport *viewport, void *renderer) {
        auto &ui = *static_cast<VKImGui *>(renderer);
        if (!ui.skippedWindows.count(viewport->ID)) ImGui_ImplVulkan_SwapBuffers(viewport, nullptr);
    }

    void VKImGui::RenderPlatformWindows() {
        skippedWindows.clear();
        ImGui::RenderPlatformWindowsDefault(nullptr, this);
        if (ImGui::GetPlatformIO().Viewports.Size > 1)
            CheckImGuiResult(vkQueueWaitIdle(device->getGraphicsQueue().getQueue()));
    }
}