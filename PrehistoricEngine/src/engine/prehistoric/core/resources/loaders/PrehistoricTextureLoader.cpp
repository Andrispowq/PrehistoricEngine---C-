#include "Includes.hpp"
#include "PrehistoricTextureLoader.h"

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "prehistoric/application/Application.h"

#include "TextureLoader.h"

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
			uint8_t* arr = new uint8_t[size];
			stream.read((char*)arr, size);

			RawTextureData* raw_data = (RawTextureData*)arr;

			int width, height, channels, type;
			uint8_t* data = new uint8_t[raw_data->size];
			
			width = raw_data->width;
			height = raw_data->height;
			channels = raw_data->channels;
			type = raw_data->imageType;
			memcpy(data, &raw_data->data, raw_data->size);

			if (data == nullptr)
			{
				PR_LOG_ERROR("Failed to load texture %s!\n", location.c_str());
			}

			ImageData::ImagePtr ptr;
			ptr.dataUC = data;

			images[i] = { ImageData::ImageType::LDR, width, height, channels, data };

			delete[] arr;
		}
	}

	PrehistoricTextureLoader::PrehistoricTextureLoader(Window* window)
		: Loader(window)
	{
	}

	void* PrehistoricTextureLoader::LoadResourceInternal(const std::string& path, Extra* extra)
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

		delete[] data.ptr.dataUC;

		return texture;
	}

	void PrehistoricTextureLoader::LoadResources()
	{
		size_t jobs = (int)loadQueue.size();

		PR_PROFILE("PrehistoricTextureLoader::LoadResources() for " + std::to_string(jobs) + " jobs");

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
			else if (__FrameworkConfig.api == Vulkan)
			{
				texture = new VKTexture((VKDevice*)window->getContext()->getDevices(), data_.width, data_.height);
				texture->Bind();
				texture->UploadTextureData(data_);
			}

			texture->SamplerProperties(ext->filter, ext->wrapMode);
			texture->Unbind();

			delete[] data_.ptr.dataUC;

			pointers.push_back(texture);

			if (heap)
				delete ext;
		}

		loadQueue.clear();
		threads.clear();
	}

	Texture* PrehistoricTextureLoader::LoadTexture(Window* window, const std::string& path, SamplerFilter filter, TextureWrapMode wrapMode)
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

		delete[] data.ptr.dataF;
		return texture;
	}

	ImageData PrehistoricTextureLoader::LoadTextureData(const std::string& path)
	{
		::Prehistoric::Timer t(std::string("Prehistoric::PrehistoricTextureLoader::LoadTextureData(), path: " + path), ::Prehistoric::Log::getFile());
		//PR_PROFILE(std::string("Prehistoric::TextureLoader::LoadTextureData(), path: " + path));

		std::ifstream stream(path, std::ios::ate | std::ios::binary);
		if (!stream.is_open())
			PR_LOG_RUNTIME_ERROR("Texture file %s couldn't be opened!\n", path.c_str());

		ImageData ret;

		size_t size = (size_t)stream.tellg();
		stream.seekg(0);
		uint8_t* arr = new uint8_t[size];
		stream.read((char*)arr, size);

		RawTextureData* raw_data = (RawTextureData*)arr;

		int width, height, channels, type;
		uint8_t* data = new uint8_t[raw_data->size];

		width = raw_data->width;
		height = raw_data->height;
		channels = raw_data->channels;
		type = raw_data->imageType;
		memcpy(data, &raw_data->data, raw_data->size);

		if (data == nullptr)
		{
			PR_LOG_ERROR("Failed to load texture %s!\n", path.c_str());
		}

		if (type == 1)
		{
			ImageData::ImagePtr ptr;
			ptr.dataF = (float*)data;

			ret = { ImageData::ImageType::HDR, width, height, channels, ptr };
		}
		else
		{
			ImageData::ImagePtr ptr;
			ptr.dataUC = data;

			ret = { ImageData::ImageType::LDR, width, height, channels, data };
		}

		delete[] arr;
		return ret;
	}

	bool PrehistoricTextureLoader::SavePrehistoricTexture(const std::string& path, RawTextureData* data)
	{
		std::vector<std::string> split = Util::Split(path, '/');

		std::ofstream file("res/textures/prehistoric/" + split[split.size() - 1], std::ios::binary);

		if (file.is_open())
		{
			file.write((const char*)data, data->size + 20);
			file.close();

			return true;
		}

		return false;
	}
};