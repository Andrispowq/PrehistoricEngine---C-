#ifndef VK_UTIL_H
#define VK_UTIL_H

#include "platform/vulkan/framework/surface/VKSurface.h"

#include "Includes.hpp"
#include "prehistoric/core/config/FrameworkConfig.h"

namespace Prehistoric
{
	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t presentFamily;

		bool hasGraphicsFamily;
		bool hasPresentFamily;
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VKDevice;

	static VkCommandPool copyCommandPool;

	namespace VKUtil
	{
		void Init(VkPhysicalDevice physicalDevice, VkDevice device);
		void CleanUp(VkDevice device);

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

		//Functions for swapchain creation
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		std::vector<VkFormat> GetAvailableFormats(VkPhysicalDevice physicalDevice);

		//Functions for buffer allocation
		uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer buff);
		void CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		//Image manipulation
		void TransitionImageLayout(VKDevice* device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VKDevice* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void GenerateMipmaps(VkPhysicalDevice physicalDevice, VKDevice* device, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);

		void CreateImage(VkPhysicalDevice physicalDevice, VKDevice* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
			VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkImage& image, VkDeviceMemory& memory);
		void CreateImageView(VKDevice* device, VkImage image, VkFormat format, VkImageAspectFlagBits aspect, uint32_t mipLevels, VkImageView& imageView);
	};
};

#endif