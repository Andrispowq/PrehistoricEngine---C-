#ifndef VK_ENVIRONMENT_BACKGROUND_H
#define VK_ENVIRONMENT_BACKGROUND_H

#include <memory>
#include <vulkan/vulkan.h>

namespace Prehistoric {
    class Window;
    class Camera;
    class VKDevice;
    class VKTexture;
    class VKSwapchain;

    // Direct equirectangular background; no cubemap conversion or IBL resources.
    class VKEnvironmentBackground {
    public:
        explicit VKEnvironmentBackground(Window *window);

        ~VKEnvironmentBackground();

        void RecreatePipeline();

        void Draw(VkCommandBuffer commandBuffer, const Camera *camera) const;

        VkDescriptorImageInfo getSource() const;

    private:
        VKDevice *device;
        VKSwapchain *swapchain;

        std::unique_ptr<VKTexture> texture;
        VkSampler sampler = VK_NULL_HANDLE;

        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
}

#endif