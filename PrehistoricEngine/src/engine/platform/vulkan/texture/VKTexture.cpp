#include "Includes.hpp"
#include "VkTexture.h"

namespace Prehistoric
{
	VKTexture::VKTexture(VKDevice* device, uint32_t width, uint32_t height, ImageFormat format, ImageType type)
		: Texture(width, height, format, type), device(device), image{ nullptr }
	{
		uint32_t mipLevels = (uint32_t)(std::floor(std::log2(max(width, height)))) + 1;

		image = std::make_unique<VKImage>(device, width, height, mipLevels, VKImage::getFormat(format), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VKTexture::VKTexture(VKDevice* device)
		: device(device)
	{
	}

	VKTexture::~VKTexture()
	{
		vkDestroySampler(device->getDevice(), sampler, nullptr);
	}

	void VKTexture::UploadTextureData(ImageData data)
	{
		image->UploadImageData(data);

		//Creating the sampler, which is based on bilinear sampling with 16x anisotropy filtering, which is a mode likely to be used, but at any time it can be
		//recreate with the method SamplerProperties(SamplerFilter, TextureWrapMode)
		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.anisotropyEnable = VK_TRUE;
		samplerCI.maxAnisotropy = 16;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = VK_FALSE;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.minLod = 0;
		samplerCI.maxLod = (float)image->getMipLevels();
		samplerCI.mipLodBias = 0;

		if (vkCreateSampler(device->getDevice(), &samplerCI, nullptr, &sampler) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create texture sampler!\n");
		}
	}

	void VKTexture::SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode, bool generate_mipmaps)
	{
		//We recreate the sampler
		vkDestroySampler(device->getDevice(), sampler, nullptr);

		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = VK_FALSE;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;

		switch (filter)
		{
		case Nearest:
			samplerCI.magFilter = VK_FILTER_NEAREST;
			samplerCI.minFilter = VK_FILTER_NEAREST;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCI.minLod = 0;
			samplerCI.maxLod = 0;
			samplerCI.mipLodBias = 0;
			samplerCI.anisotropyEnable = VK_FALSE;
			samplerCI.maxAnisotropy = 1;
			break;
		case Bilinear:
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCI.minLod = 0;
			samplerCI.maxLod = 0;
			samplerCI.mipLodBias = 0;
			samplerCI.anisotropyEnable = VK_FALSE;
			samplerCI.maxAnisotropy = 1;
			break;
		case Trilinear:
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.minLod = 0;
			samplerCI.maxLod = (float)image->getMipLevels();
			samplerCI.mipLodBias = 0;
			samplerCI.anisotropyEnable = VK_FALSE;
			samplerCI.maxAnisotropy = 1;
			break;
		case Anisotropic:
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.minLod = 0;
			samplerCI.maxLod = (float)image->getMipLevels();
			samplerCI.mipLodBias = 0;
			samplerCI.anisotropyEnable = VK_TRUE;
			samplerCI.maxAnisotropy = 16;
			break;
		default:
			break;
		}

		switch (wrapMode)
		{
		case ClampToEdge:
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case ClampToBorder:
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		case Repeat:
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case MirrorRepeat:
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		default:
			break;
		}

		if (vkCreateSampler(device->getDevice(), &samplerCI, nullptr, &sampler) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("Failed to create texture sampler!\n");
		}
	}
};