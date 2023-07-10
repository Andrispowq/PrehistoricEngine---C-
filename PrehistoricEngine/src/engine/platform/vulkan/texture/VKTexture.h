#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include "prehistoric/common/texture/Texture.h"

#include <vulkan/vulkan.h>

#include "VKImage.h"

#include "platform/vulkan/framework/device/VKDevice.h"

namespace Prehistoric
{
	class VKTexture : public Texture
	{
	public:
		VKTexture(VKDevice* device, uint32_t width, uint32_t height, ImageFormat format = R8G8B8A8_LINEAR, ImageType type = TEXTURE_2D);
		VKTexture(VKDevice* device);

		virtual ~VKTexture();

		virtual void Bind(uint32_t slot = 0) const override {}
		virtual void BindImage(uint32_t slot = 0) const override {}
		virtual void Unbind() const override {}

		virtual void UploadTextureData(ImageData data) override;
		virtual void SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps = true) override;

		VkSampler& getSampler() { return sampler; };
		VKImage* getImage() { return image.get(); }

	private:
		VKDevice* device;
		
		std::unique_ptr<VKImage> image;
		VkSampler sampler;
	};
};

#endif