#ifndef VK_ENVIRONMENT_BACKGROUND_H
#define VK_ENVIRONMENT_BACKGROUND_H

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace Prehistoric {
    class Window;
    class Camera;
    class Atmosphere;
    class VKDevice;
    class VKTexture;
    class VKSwapchain;

    // Owns the HDR input and draws the baked reflection cubemap.
    class VKEnvironmentBackground {
    public:
        explicit VKEnvironmentBackground(Window *window);

        ~VKEnvironmentBackground();

        void SetPrefilter(const VkDescriptorImageInfo &imageInfo);

        void RecreatePipeline();

        void Draw(VkCommandBuffer commandBuffer, const Camera *camera, const Atmosphere *atmosphere, float lod) const;

        VkDescriptorImageInfo LoadSource(Window *window, const std::string& location);

    private:
        VKDevice *device;
        VKSwapchain *swapchain;

        std::string sourceLocation;
        std::unique_ptr<VKTexture> texture;
        VkSampler sampler = VK_NULL_HANDLE;

        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline atmospherePipeline = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
}

#endif