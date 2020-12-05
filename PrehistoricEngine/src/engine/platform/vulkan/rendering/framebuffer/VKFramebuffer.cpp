#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKFramebuffer.h"
#include "engine/platform/vulkan/rendering/renderpass/VKRenderpass.h"

namespace Prehistoric
{
	VKFramebuffer::VKFramebuffer(VkDevice device, VkRenderPass renderpass, uint32_t width, uint32_t height, size_t imageViewCount, VkImageView imageViews[])
		: device(device)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderpass;
		framebufferInfo.attachmentCount = (uint32_t)imageViewCount;
		framebufferInfo.pAttachments = imageViews;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create framebuffer!\n");
		}
	}

	VKFramebuffer::~VKFramebuffer()
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
};
