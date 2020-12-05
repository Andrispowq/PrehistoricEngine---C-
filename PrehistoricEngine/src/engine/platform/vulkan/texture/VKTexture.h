#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include "prehistoric/common/texture/Texture.h"

#include <vulkan/vulkan.h>

#include "platform/vulkan/framework/device/VKDevice.h"

namespace Prehistoric
{
	class VKTexture : public Texture
	{
	public:
		VKTexture(VKPhysicalDevice* physicalDevice, VKDevice* device, uint32_t width, uint32_t height, ImageFormat format = R8G8B8A8_LINEAR, ImageType type = TEXTURE_2D);
		VKTexture(VKPhysicalDevice* physicalDevice, VKDevice* device);

		virtual ~VKTexture();

		virtual void Bind(uint32_t slot = 0) const override {}
		virtual void Unbind() const override {}

		virtual void UploadTextureData(unsigned char* pixels, ImageFormat format) override;
		virtual void Generate() override;

		virtual void SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode) override;

		VkImageView& getTextureImageView() { return textureImageView; }
		VkSampler& getTextureSampler() { return textureSampler; }
	private:
		VkFormat getFormat(ImageFormat format) const;
	private:
		VKDevice* device;
		VKPhysicalDevice* physicalDevice;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;

		VkImageView textureImageView;
		VkSampler textureSampler;

		uint32_t mipLevels;
	};
};

#endif