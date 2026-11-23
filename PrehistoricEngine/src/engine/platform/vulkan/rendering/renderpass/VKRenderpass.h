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

		std::unique_ptr<VKFramebuffer> CreateFramebuffer(uint32_t imageIndex) const;
		void BeginRenderpass(VKCommandBuffer* commandBuffer, VKFramebuffer* framebuffer, VkExtent2D swapchainExtent, Vector4f clearColour);
		void RenderHDR(VKCommandBuffer* commandBuffer);

		VkPipeline CreateFullscreenPipeline(const char* vertexPath, const char* fragmentPath, VkPipelineLayout layout, uint32_t subpass);
		VkRenderPass getRenderPass() const { return renderpass; }
	private:
		VkRenderPass renderpass;
		VkDescriptorSetLayout hdrSetLayout{};
		VkDescriptorPool hdrPool{};
		VkDescriptorSet hdrSet{};
		VkPipelineLayout hdrLayout{};
		VkPipeline hdrPipeline{};

		VKDevice* device;
		VKSwapchain* swapchain;
	};
};

#endif