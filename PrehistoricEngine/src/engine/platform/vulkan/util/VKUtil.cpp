#include "Includes.hpp"
#include <glad/glad.h>
#include "VKUtil.h"
#include "platform/vulkan/framework/device/VKDevice.h"
#include "platform/vulkan/rendering/pipeline/VKPipeline.h"

namespace Prehistoric
{
	namespace VKUtil
	{
		void VKUtil::Init(VkPhysicalDevice physicalDevice, VkDevice device)
		{
			VkCommandPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

			if (vkCreateCommandPool(device, &info, nullptr, &copyCommandPool) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to create copy command pool!\n");
			}
		}

		void VKUtil::CleanUp(VkDevice device)
		{
			vkDestroyCommandPool(device, copyCommandPool, nullptr);
		}

		QueueFamilyIndices VKUtil::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
		{
			QueueFamilyIndices indices = { 0, false };

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;

			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
					indices.hasGraphicsFamily = true;
				}

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

				if (presentSupport)
				{
					indices.presentFamily = i;
					indices.hasPresentFamily = true;
				}

				if (indices.hasGraphicsFamily && indices.hasPresentFamily)
				{
					break;
				}

				i++;
			}

			return indices;
		}

		SwapChainSupportDetails VKUtil::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
		{
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		VkSurfaceFormatKHR VKUtil::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR VKUtil::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D VKUtil::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != UINT32_MAX)
			{
				return capabilities.currentExtent;
			}
			else
			{
				VkExtent2D actualExtent = { FrameworkConfig::windowWidth, FrameworkConfig::windowHeight };

				actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		VkFormat VKUtil::FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
		{
			for (VkFormat format : candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				{
					return format;
				}
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}

			PR_LOG_RUNTIME_ERROR("Couldn't find suitable format!\n");
			return VK_FORMAT_R8G8B8A8_SRGB;
		}

		uint32_t VKUtil::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			PR_LOG_RUNTIME_ERROR("Failed to find suitable memory type!\n");
			return 0;
		}

		void VKUtil::CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to create buffer!\n");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to allocate buffer memory!\n");
			}

			vkBindBufferMemory(device, buffer, bufferMemory, 0);
		}

		void VKUtil::CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			VkCommandBufferAllocateInfo copyBufferAI = {};
			copyBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			copyBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			copyBufferAI.commandPool = copyCommandPool;
			copyBufferAI.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device->getDevice(), &copyBufferAI, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;

			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getGraphicsQueue().getQueue());

			vkFreeCommandBuffers(device->getDevice(), copyCommandPool, 1, &commandBuffer);
		}

		void VKUtil::CopyBuffer(VKDevice* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer buffer)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(buffer, &beginInfo);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;

			vkCmdCopyBuffer(buffer, srcBuffer, dstBuffer, 1, &copyRegion);

			vkEndCommandBuffer(buffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &buffer;

			vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getGraphicsQueue().getQueue());
		}

		void VKUtil::TransitionImageLayout(VKDevice* device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = copyCommandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				//The image has stencil component, if this is true:
				if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
				{
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				PR_LOG_RUNTIME_ERROR("Unsupported image layout transition!\n");
			}

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getGraphicsQueue().getQueue());

			vkFreeCommandBuffers(device->getDevice(), copyCommandPool, 1, &commandBuffer);
		}

		void VKUtil::CopyBufferToImage(VKDevice* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = copyCommandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getGraphicsQueue().getQueue());

			vkFreeCommandBuffers(device->getDevice(), copyCommandPool, 1, &commandBuffer);
		}

		void VKUtil::GenerateMipmaps(VkPhysicalDevice physicalDevice, VKDevice* device, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

			if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			{
				PR_LOG_RUNTIME_ERROR("Texture image format does not support linear blitting!");
			}

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = copyCommandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = (int32_t)width;
			int32_t mipHeight = (int32_t)height;

			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				VkImageBlit blit = {};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(commandBuffer,
					image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(device->getGraphicsQueue().getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getGraphicsQueue().getQueue());

			vkFreeCommandBuffers(device->getDevice(), copyCommandPool, 1, &commandBuffer);
		}

		std::vector<VkFormat> VKUtil::GetAvailableFormats(VkPhysicalDevice physicalDevice)
		{
			//TODO: query available VkFormats
			return {};
		}

		void VKUtil::CreateImage(VkPhysicalDevice physicalDevice, VKDevice* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples,
			VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkImage& image, VkDeviceMemory& memory)
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = mipLevels;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = samples;
			imageInfo.flags = 0; // Optional

			if (vkCreateImage(device->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to create image!\n");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device->getDevice(), image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = VKUtil::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, memoryFlags);

			if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to allocate image memory!\n");
			}

			vkBindImageMemory(device->getDevice(), image, memory, 0);
		}

		void VKUtil::CreateImageView(VKDevice* device, VkImage image, VkFormat format, VkImageAspectFlagBits aspect, uint32_t mipLevels, VkImageView& imageView)
		{
			VkImageViewCreateInfo colorImageViewCreateInfo = {};
			colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageViewCreateInfo.image = image;
			colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageViewCreateInfo.format = format;

			colorImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			colorImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			colorImageViewCreateInfo.subresourceRange.aspectMask = aspect;
			colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			colorImageViewCreateInfo.subresourceRange.levelCount = mipLevels;
			colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			colorImageViewCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device->getDevice(), &colorImageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
			{
				PR_LOG_RUNTIME_ERROR("Failed to create image views!\n");
			}
		}
	};
};