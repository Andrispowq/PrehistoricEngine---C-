#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/framebuffer/VKFramebuffer.h"
#include "platform/vulkan/rendering/command/VKCommandBuffer.h"

namespace Prehistoric
{
	class VKSwapchain;

	class VKRenderpass
	{
	public:
		VKRenderpass(VKDevice* device, VKSwapchain* swapchain);
		virtual ~VKRenderpass();

		void BeginRenderpass(VKCommandBuffer* commandBuffer, VKFramebuffer* framebuffer, VkExtent2D swapchainExtent, Vector4f clearColour);
		void EndRenderpass(VKCommandBuffer* commandBuffer);

		VkRenderPass getRenderPass() const { return renderpass; }
	private:
		VkRenderPass renderpass;

		VKDevice* device;
		VKSwapchain* swapchain;
	};
};

#endif