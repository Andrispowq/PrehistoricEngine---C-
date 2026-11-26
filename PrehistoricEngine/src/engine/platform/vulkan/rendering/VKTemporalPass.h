#pragma once
#include "platform/vulkan/texture/VKImage.h"
#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"
#include "platform/vulkan/buffer/VKBuffer.h"

namespace Prehistoric {
    class VKRenderer;
    class VKRenderpass;

    class VKTemporalPass {
    public:
        explicit VKTemporalPass(VKRenderpass &owner);

        ~VKTemporalPass();

        VkImageView Render(VKRenderer &renderer);

    private:
        VKRenderpass &owner;
        VkDevice device{};
        VkExtent2D extent{};
        VkFormat motionFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkRenderPass velocityPass{}, motionPass{};
        VkDescriptorSetLayout setLayout{};
        VkPipelineLayout layout{}, geometryLayout{};
        VkPipeline velocities[3]{}, resolve{}, filter{};
        VkDescriptorPool pool{};
        std::unique_ptr<VKImage> images[8], depth;
        std::unique_ptr<VKFramebuffer> frames[8];

        struct Frame {
            std::unique_ptr<VKBuffer> uniforms;
            VkDescriptorSet sets[5]{};
            uint64_t timingFrame = 0;
            unsigned timingMode = 0;
        };

        std::vector<Frame> slots;
        VkQueryPool timers{};
        double timestampMs = 0;
        uint64_t timestampMask = 0;
        std::vector<bool> queried;
        bool initialized = false;

        void CreateVelocityPipeline();
    };
} // namespace Prehistoric