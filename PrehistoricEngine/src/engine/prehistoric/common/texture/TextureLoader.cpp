#include "Includes.hpp"
#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

namespace Prehistoric
{
	namespace TextureLoader
	{
		Texture* TextureLoader::LoadTexture(const std::string& path, Window* window, SamplerFilter filter, TextureWrapMode wrapMode)
		{
			Texture* texture = nullptr;
			ImageData data = LoadTextureData(path);

			if (data.type == ImageData::ImageType::HDR)
			{
				texture = new GLTexture(data.width, data.height);
				texture->Bind();
				static_cast<GLTexture*>(texture)->UploadHDRTextureData(data.ptr.dataF);
			}
			else
			{
				ImageFormat format;
				if (data.channels == 3 && FrameworkConfig::api != Vulkan) //Vulkan can't decide if it supports 24 bit textures yet, so don't do this
					format = R8G8B8_LINEAR;
				else
					format = R8G8B8A8_LINEAR;

				if (FrameworkConfig::api == OpenGL)
				{
					texture = new GLTexture(data.width, data.height);
					texture->Bind();
					texture->UploadTextureData(data.ptr.dataUC, format);
				}
				else if (FrameworkConfig::api == Vulkan)
				{
					texture = new VKTexture((VKPhysicalDevice*)window->getContext()->getPhysicalDevice(), (VKDevice*)window->getContext()->getDevice(), data.width, data.height);
					texture->Bind();
					texture->UploadTextureData(data.ptr.dataUC, format);
					texture->Generate();
				}
			}

			texture->SamplerProperties(filter, wrapMode);
			texture->Unbind();

			stbi_image_free(data.ptr.dataF);
			return texture;
		}

		ImageData TextureLoader::LoadTextureData(const std::string& path)
		{
			PR_PROFILE(std::string("Prehistoric::TextureLoader::LoadTextureData(std::string&), path: " + path));
			if (FrameworkConfig::api == OpenGL)
				stbi_set_flip_vertically_on_load(1);

			std::ifstream stream(path, std::ios::ate | std::ios::binary);
			size_t size = (size_t)stream.tellg();
			stream.seekg(0);
			unsigned char* buffer = new unsigned char[size];
			stream.read((char*)buffer, size);

			ImageData ret;

			int width, height, channels;
			if (stbi_is_hdr(path.c_str()))
			{
				float* data = stbi_loadf_from_memory(buffer, (int)size, &width, &height, &channels, 0);
				//float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

				if (data == nullptr)
				{
					PR_LOG_ERROR("Failed to load texture %s!\n", path.c_str());
				}

				ImagePtr ptr;
				ptr.dataF = data;

				ret = { ImageData::ImageType::HDR, width, height, channels, ptr };
			}
			else
			{
				unsigned char* data;
				if (FrameworkConfig::api == OpenGL)
				{
					data = stbi_load_from_memory(buffer, (int)size, &width, &height, &channels, 0);
					//data = stbi_load(path.c_str(), &width, &height, &channels, 0);
				}
				else
				{
					data = stbi_load_from_memory(buffer, (int)size, &width, &height, &channels, STBI_rgb_alpha);
					//data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
				}

				if (data == nullptr)
				{
					PR_LOG_ERROR("Failed to load texture %s!\n", path.c_str());
				}

				ImagePtr ptr;
				ptr.dataUC = data;

				ret = { ImageData::ImageType::LDR, width, height, channels, data };
			}

			delete[] buffer;
			return ret;
		}
	};
};