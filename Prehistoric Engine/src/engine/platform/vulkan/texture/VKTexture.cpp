#include "engine/prehistoric/core/util/Includes.hpp"
#include "VkTexture.h"

VKTexture::VKTexture(VKPhysicalDevice* physicalDevice, VKDevice* device, uint32_t width, uint32_t height, ImageFormat format, ImageType type)
	: physicalDevice(physicalDevice), device(device), Texture(width, height, format, type)
{
	this->mipLevels = (uint32_t)(std::floor(std::log2(std::max(width, height)))) + 1;
}

VKTexture::VKTexture(VKPhysicalDevice* physicalDevice, VKDevice* device)
	: physicalDevice(physicalDevice), device(device), mipLevels(1)
{
}

VKTexture::~VKTexture()
{
	vkDestroySampler(device->getDevice(), textureSampler, nullptr);
	vkDestroyImageView(device->getDevice(), textureImageView, nullptr);

	vkDestroyImage(device->getDevice(), textureImage, nullptr);
	vkFreeMemory(device->getDevice(), textureImageMemory, nullptr);
}

void VKTexture::UploadTextureData(unsigned char* pixels, ImageFormat format)
{
	uint8_t channels = (format == R8G8B8_LINEAR || format == R8G8B8_SRGB) ? 3 : 4;
	size_t size = width * height * channels;

	//TODO: hardcoded 4 should be removed
 	VKUtil::CreateBuffer(physicalDevice->getPhysicalDevice(), device->getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	this->format = format;

	void* data;
	vkMapMemory(device->getDevice(), stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, pixels, size);
	vkUnmapMemory(device->getDevice(), stagingBufferMemory);
}

void VKTexture::Generate()
{
	//We can create the image new based on the staging buffer's data
	VKUtil::CreateImage(physicalDevice->getPhysicalDevice(), device, width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT, getFormat(format), VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		textureImage, textureImageMemory);

	VKUtil::TransitionImageLayout(device, textureImage, /*GetFormat(format)*/VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	VKUtil::CopyBufferToImage(device, stagingBuffer, textureImage, width, height);
	//VKUtil::TransitionImageLayout(*device, textureImage, GetFormat(format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
	VKUtil::GenerateMipmaps(physicalDevice->getPhysicalDevice(), device, textureImage, getFormat(format), width, height, mipLevels);
	
	//We're no longer in need of the staging buffers
	vkDestroyBuffer(device->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device->getDevice(), stagingBufferMemory, nullptr);

	//We can now create the image view
	VKUtil::CreateImageView(device, textureImage, getFormat(format), VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, textureImageView);
	
	//Creating the sampler, which is based on bilinear sampling with 16x anisotropy filtering, which is a mode likely to be used, but at any time it can be
	//recreate with the method SamplerProperties(SamplerFilter, TextureWrapMode)
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = (float) mipLevels;
	samplerInfo.mipLodBias = 0;

	if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) 
	{
		PR_LOG_RUNTIME_ERROR("Failed to create texture sampler!\n");
	}
}

void VKTexture::SamplerProperties(SamplerFilter filter, TextureWrapMode wrapMode)
{
	//We recreate the sampler
	vkDestroySampler(device->getDevice(), textureSampler, nullptr);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	switch (filter)
	{
	case Nearest:
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 0;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		break;
	case Bilinear:
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 0;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		break;
	case Trilinear:
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = (float)mipLevels;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		break;
	case Anisotropic:
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = (float)mipLevels;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		break;
	default:
		break;
	}

	switch (wrapMode)
	{
	case ClampToEdge:
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case ClampToBorder:
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	case Repeat:
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case MirrorRepeat:
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		break;
	default:
		break;
	}

	if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		PR_LOG_RUNTIME_ERROR("Failed to create texture sampler!\n");
	}
}

VkFormat VKTexture::getFormat(ImageFormat format) const
{
	//TODO: See which formats are available, return if it is, and return VK_FORMAT_R8G8B8A8_SRGB if it's not
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