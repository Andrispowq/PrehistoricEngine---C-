#ifndef VK_UTIL_H
#define VK_UTIL_H

#include "platform/vulkan/framework/surface/VKSurface.h"
#include "platform/vulkan/framework/device/VKDevice.h"

namespace Prehistoric
{
	class VKUtil
	{
	public:
		static void Init(VKDevice* device);
		static void CleanUp(VKDevice* device);

		//Functions for allocations and buffer copying
		static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer buff);
		static void CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		//Image manipulation
		static void TransitionImageLayout(VKDevice* device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		static void CopyBufferToImage(VKDevice* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		static void GenerateMipmaps(VKDevice* device, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);

	private:
		static VkCommandPool copyCommandPool;
	};
};

#endif