#include "Includes.hpp"
#include "VKImage.h"

#include "platform/vulkan/util/VKUtil.h"

#include "platform/vulkan/buffer/VKBuffer.h"

namespace Prehistoric
{
	VKImage::VKImage(VKDevice* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspect)
		: device(device), mipLevels(mipLevels), format(format)
	{
		//Create the image
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.extent.width = width;
		imageCI.extent.height = height;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = mipLevels;
		imageCI.arrayLayers = 1;
		imageCI.format = format;
		imageCI.tiling = tiling;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = usage;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.samples = samples;
		imageCI.flags = 0; // Optional

		if (vkCreateImage(device->getDevice(), &imageCI, nullptr, &image) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create an image!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device->getDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VKUtil::FindMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, memoryFlags);

		if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to allocate image memory!\n");
		}

		vkBindImageMemory(device->getDevice(), image, imageMemory, 0);

		//Create the image view
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = format;

		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCI.subresourceRange.aspectMask = aspect;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = mipLevels;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device->getDevice(), &imageViewCI, nullptr, &imageView) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create an image view!\n");
		}
	}

	VKImage::VKImage(VKDevice* device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlagBits aspect)
		: device(device), image(image), mipLevels(mipLevels), format(format)
	{
		//Create the image view
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = format;

		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCI.subresourceRange.aspectMask = aspect;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = mipLevels;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device->getDevice(), &imageViewCI, nullptr, &imageView) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create an image view!\n");
		}
	}

	VKImage::~VKImage()
	{
		vkDestroyImageView(device->getDevice(), imageView, nullptr);

		vkDestroyImage(device->getDevice(), image, nullptr);
		vkFreeMemory(device->getDevice(), imageMemory, nullptr);
	}

	void VKImage::UploadImageData(ImageData data)
	{
		this->width = data.width;
		this->height = data.height;

		//TODO:
		data.channels = 4;

		VKBuffer stagingBuffer(device, data.width * data.height * data.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.MapMemory();
		stagingBuffer.Store(data.ptr.dataUC);
		stagingBuffer.UnmapMemory();

		VKUtil::TransitionImageLayout(device, image, /*GetFormat(format)*/VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		VKUtil::CopyBufferToImage(device, stagingBuffer.getBuffer(), image, width, height);
		
		if (mipLevels < 2)
		{
			VKUtil::TransitionImageLayout(device, image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
		}
		else
		{
			VKUtil::GenerateMipmaps(device, image, format, width, height, mipLevels);
		}
	}

	VkFormat VKImage::getFormat(ImageFormat format)
	{
		switch (format)
		{
		case R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case R8G8B8_SRGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case D32_SFLOAT:
			return VK_FORMAT_D32_SFLOAT;
		case R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;
		case R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case R8_SRGB:
			return VK_FORMAT_R8_SRGB;
		case R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;
		case R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case R8G8B8A8_LINEAR:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case R8G8B8_LINEAR:
			return VK_FORMAT_R8G8B8_UNORM;
		case D32_LINEAR:
			return VK_FORMAT_D32_SFLOAT_S8_UINT; //Not sure about that one
		case R16_LINEAR:
			return VK_FORMAT_R16_UNORM;
		case R32_LINEAR:
			return VK_FORMAT_R32_UINT;
		case R8_LINEAR:
			return VK_FORMAT_R8_UNORM;
		case R8G8_LINEAR:
			return VK_FORMAT_R8G8_UNORM;
		case R16G16_LINEAR:
			return VK_FORMAT_R16G16_UNORM;
		default:
			return VK_FORMAT_R8G8B8A8_SRGB;
		}
	}
};
