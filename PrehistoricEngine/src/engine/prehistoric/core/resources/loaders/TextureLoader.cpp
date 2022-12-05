#include "Includes.hpp"
#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "prehistoric/application/Application.h"

#include "PrehistoricTextureLoader.h"

namespace Prehistoric
{
	static ImageData images[NUM_THREADS];

	static void load_textures(size_t start, size_t count, std::string locations[])
	{
		for (size_t i = start; i < start + count; i++)
		{
			std::string location = locations[i];
			std::ifstream stream(location, std::ios::ate | std::ios::binary);
			if (!stream.is_open())
				PR_LOG_RUNTIME_ERROR("Texture file %s couldn't be opened!\n", location.c_str());

			size_t size = (size_t)stream.tellg();
			stream.seekg(0);
			unsigned char* arr = new unsigned char[size];
			stream.read((char*)arr, size);

			int width, height, channels;
			unsigned char* data;
			if (__FrameworkConfig.api == OpenGL)
			{
				data = stbi_load_from_memory(arr, (int)size, &width, &height, &channels, 0);
			}
			else
			{
				data = stbi_load_from_memory(arr, (int)size, &width, &height, &channels, STBI_rgb_alpha);
			}

			if (data == nullptr)
			{
				PR_LOG_ERROR("Failed to load texture %s!\n", location.c_str());
			}

			ImageData::ImagePtr ptr;
			ptr.dataUC = data;

			images[i] = { ImageData::ImageType::LDR, width, height, channels, data };

			uint32_t struct_size = width * height * channels / 4 + 20;
			uint8_t* bytes = new uint8_t[struct_size];
			memset(bytes, 0, struct_size);

			RawTextureData* raw_data = (RawTextureData*)bytes;
			raw_data->width = width;
			raw_data->height = height;
			raw_data->channels = channels;
			raw_data->imageType = 0;
			raw_data->size = struct_size - 20;
			memcpy(raw_data->data, data, raw_data->size);
			PrehistoricTextureLoader::SavePrehistoricTexture(location.substr(0, location.length() - 3) + "prt", raw_data);
			delete bytes;

			delete[] arr;
		}
	}

	TextureLoader::TextureLoader(Window* window)
		: Loader(window)
	{
		if (__FrameworkConfig.api == OpenGL)
			stbi_set_flip_vertically_on_load(1);
	}

	void* TextureLoader::LoadResourceInternal(const std::string& path, Extra* extra)
	{
		TextureLoaderExtra* ext = dynamic_cast<TextureLoaderExtra*>(extra);
		Texture* texture = nullptr;
		ImageData data = LoadTextureData(path);

		if (data.type == ImageData::ImageType::HDR)
		{
			data.channels = 4; //We use compute shaders with HDR textures, and it currently only works with RGBA textures
		}

		if (__FrameworkConfig.api == OpenGL)
		{
			texture = new GLTexture(data.width, data.height);
			texture->Bind();
			texture->UploadTextureData(data);
		}
		else if (__FrameworkConfig.api == Vulkan)
		{
			texture = new VKTexture((VKDevice*)window->getContext()->getDevices(), data.width, data.height);
			texture->Bind();
			texture->UploadTextureData(data);
		}

		texture->SamplerProperties(ext->filter, ext->wrapMode);
		texture->Unbind();

		stbi_image_free(data.ptr.dataF);

		return texture;
	}

	void TextureLoader::LoadResources()
	{
		size_t jobs = (int)loadQueue.size();

		PR_PROFILE("TextureLoader::LoadResources() for " + std::to_string(jobs) + " jobs");

		threads.reserve(NUM_THREADS);

		size_t threadCount = 0;

		std::string* locations = new std::string[jobs];
		for (size_t i = 0; i < jobs; i++)
		{
			auto [name, extra, heap] = loadQueue[i];
			locations[i] = name;
		}

		for (size_t i = 0; i < NUM_THREADS; i++)
		{
			if (jobs == 0)
				break;

			if (jobs >= 4)
			{
				threads.emplace_back(load_textures, size_t(i * 4), (size_t)4, locations);
				jobs -= 4;
			}
			else
			{
				threads.emplace_back(load_textures, size_t(i * 4), (size_t)jobs, locations);
				jobs = 0;
			}

			threadCount++;
		}

		for (size_t i = 0; i < threadCount; i++)
		{
			threads[i].join();
		}

		delete[] locations;
		jobs = (int)loadQueue.size();

		for (size_t i = 0; i < jobs; i++)
		{
			auto [name, extra, heap] = loadQueue[i];

			Texture* texture = nullptr;
			TextureLoaderExtra* ext = dynamic_cast<TextureLoaderExtra*>(extra);
			ImageData data_ = images[i];

			if (data_.type == ImageData::ImageType::HDR)
			{
				data_.channels = 4; //We use compute shaders with HDR textures, and it currently only works with RGBA textures
			}

			if (__FrameworkConfig.api == OpenGL)
			{
				texture = new GLTexture(data_.width, data_.height);
				texture->Bind();
				texture->UploadTextureData(data_);
			}
			else if (__FrameworkConfig.api ==  Vulkan)
			{
				texture = new VKTexture((VKDevice*)window->getContext()->getDevices(), data_.width, data_.height);
				texture->Bind();
				texture->UploadTextureData(data_);
			}

			texture->SamplerProperties(ext->filter, ext->wrapMode);
			texture->Unbind();

			stbi_image_free(data_.ptr.dataF);

			pointers.push_back(texture);
			
			if(heap)
				delete ext;
		}

		loadQueue.clear();
		threads.clear();
	}

	Texture* TextureLoader::LoadTexture(Window* window, const std::string& path, SamplerFilter filter, TextureWrapMode wrapMode)
	{
		Texture* texture = nullptr;
		ImageData data = LoadTextureData(path);

		if (data.type == ImageData::ImageType::HDR)
		{
			data.channels = 4; //We use compute shaders with HDR textures, and it currently only works with RGBA textures
		}

		if (__FrameworkConfig.api == OpenGL)
		{
			texture = new GLTexture(data.width, data.height);
			texture->Bind();
			texture->UploadTextureData(data);
		}
		else if (__FrameworkConfig.api == Vulkan)
		{
			texture = new VKTexture((VKDevice*)window->getContext()->getDevices(), data.width, data.height);
			texture->Bind();
			texture->UploadTextureData(data);
		}

		texture->SamplerProperties(filter, wrapMode);
		texture->Unbind();

		stbi_image_free(data.ptr.dataF);
		return texture;
	}

	ImageData TextureLoader::LoadTextureData(const std::string& path)
	{
		::Prehistoric::Timer t(std::string("Prehistoric::TextureLoader::LoadTextureData(), path: " + path), ::Prehistoric::Log::getFile());
		//PR_PROFILE(std::string("Prehistoric::TextureLoader::LoadTextureData(), path: " + path));

		std::ifstream stream(path, std::ios::ate | std::ios::binary);
		if (!stream.is_open())
			PR_LOG_RUNTIME_ERROR("Texture file %s couldn't be opened!\n", path.c_str());

		size_t size = (size_t)stream.tellg();
		stream.seekg(0);
		unsigned char* buffer = new unsigned char[size];
		stream.read((char*)buffer, size);

		ImageData ret;

		int width, height, channels;
		if (stbi_is_hdr(path.c_str()))
		{
			float* data = stbi_loadf_from_memory(buffer, (int)size, &width, &height, &channels, 0);

			if (data == nullptr)
			{
				PR_LOG_ERROR("Failed to load texture %s!\n", path.c_str());
			}

			ImageData::ImagePtr ptr;
			ptr.dataF = data;

			ret = { ImageData::ImageType::HDR, width, height, channels, ptr };
		}
		else
		{
			unsigned char* data;
			if (__FrameworkConfig.api == OpenGL)
			{
				data = stbi_load_from_memory(buffer, (int)size, &width, &height, &channels, 0);
			}
			else
			{
				data = stbi_load_from_memory(buffer, (int)size, &width, &height, &channels, STBI_rgb_alpha);
			}

			if (data == nullptr)
			{
				PR_LOG_ERROR("Failed to load texture %s!\n", path.c_str());
			}

			ImageData::ImagePtr ptr;
			ptr.dataUC = data;

			ret = { ImageData::ImageType::LDR, width, height, channels, data };
		}

		uint32_t struct_size = width * height * channels / 4 + 20;
		uint8_t* bytes = new uint8_t[struct_size];
		memset(bytes, 0, struct_size);

		RawTextureData* raw_data = (RawTextureData*)bytes;
		raw_data->width = width;
		raw_data->height = height;
		raw_data->channels = channels;
		raw_data->imageType = 0;
		raw_data->size = struct_size - 20;
		memcpy(raw_data->data, ret.ptr.dataUC, raw_data->size);
		PrehistoricTextureLoader::SavePrehistoricTexture(path.substr(0, path.length() - 3) + "prt", raw_data);
		delete bytes;

		return ret;
	}
};