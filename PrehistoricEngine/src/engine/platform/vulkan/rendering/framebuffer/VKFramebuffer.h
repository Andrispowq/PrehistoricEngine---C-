#ifndef VK_FRAMEBUFFER_H
#define VK_FRAMEBUFFER_H

#include <vulkan/vulkan.h>

namespace Prehistoric {
    class VKRenderpass;

    class VKFramebuffer {
    public:
        VKFramebuffer(VkDevice device, VkRenderPass renderpass, uint32_t width, uint32_t height, size_t imageViewCount,
                      VkImageView imageViews[]);

        virtual ~VKFramebuffer();

        VkExtent2D getExtent() const { return extent; }
        VkFramebuffer getFramebuffer() const { return framebuffer; }

    private:
        VkDevice device;
        VkExtent2D extent;

        VkFramebuffer framebuffer;
    };
};

#endif