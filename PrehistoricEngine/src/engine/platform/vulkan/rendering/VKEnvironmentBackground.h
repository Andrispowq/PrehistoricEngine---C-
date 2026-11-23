#ifndef VK_ENVIRONMENT_BACKGROUND_H
#define VK_ENVIRONMENT_BACKGROUND_H

#include <memory>
#include <optional>
#include "prehistoric/core/util/math/Math.h"
#include <vulkan/vulkan.h>

namespace Prehistoric {
    class Window;
    class Camera;
    class VKDevice;
    class VKTexture;
    class VKSwapchain;

    // Owns the HDR input and draws the baked reflection cubemap.
    class VKEnvironmentBackground {
    public:
        explicit VKEnvironmentBackground(Window *window);

        ~VKEnvironmentBackground();

        void SetPrefilter(const VkDescriptorImageInfo& imageInfo);
        void RecreatePipeline();

        void Draw(VkCommandBuffer commandBuffer, const Camera* camera, const std::optional<Vector3f>& sun) const;

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
        VkPipeline atmospherePipeline = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
}

#endif