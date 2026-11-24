#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <set>

struct ImGuiViewport;

namespace Prehistoric {
    class VKContext;
    class VKDevice;
    class VKSwapchain;
    class VKTexture;

    class VKImGui {
    public:
        VKImGui(VKContext *context, VKSwapchain *swapchain);

        ~VKImGui();

        void Begin();

        void Draw(VkCommandBuffer command);

        void RenderPlatformWindows();

        VkDescriptorSet BindTexture(VKTexture *texture);

    private:
        static void DrawWindow(ImGuiViewport *viewport, void *renderer);

        static void PresentWindow(ImGuiViewport *viewport, void *renderer);

        std::map<VkFormat, VkPipeline> platformPipelines;
        std::set<unsigned int> skippedWindows;

        void InitRenderer();

        void ShutdownRenderer();

        VkDescriptorPool CreatePool(uint32_t capacity);

        VKContext *context;
        VKDevice *device;
        VKSwapchain *swapchain;
        VkDescriptorPool fontPool{};
        std::vector<VkDescriptorPool> texturePools;
        uint32_t textureCount = 0;
        VkFormat format{};
        uint32_t imageCount = 0;
    };
}