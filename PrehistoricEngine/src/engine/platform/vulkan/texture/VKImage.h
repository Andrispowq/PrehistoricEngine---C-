#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/core/resources/loaders/TextureLoader.h"

namespace Prehistoric
{
	class VKImage
	{
	public:
		VKImage(VKDevice* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, 
			VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspect);
		VKImage(VKDevice* device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlagBits aspect);
		~VKImage();

		void UploadImageData(ImageData data);

		VkImageView getImageView() const { return imageView; }
		VkImage getImage() const { return image; }
		VkDeviceMemory getImageMemory() const { return imageMemory; }

		uint32_t getMipLevels() const { return mipLevels; }
		VkFormat getImageFormat() const { return format; }

		static VkFormat getFormat(ImageFormat format);

	private:
		VKDevice* device;

		uint32_t width, height, mipLevels;

		VkFormat format;
		VkImageLayout layout;

		VkImageView imageView;
		VkImage image;
		VkDeviceMemory imageMemory;
	};
};

#endif