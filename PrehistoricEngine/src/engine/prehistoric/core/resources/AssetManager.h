#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "Includes.hpp"

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/model/obj/OBJLoader.h"
#include "prehistoric/common/texture/TextureLoader.h"

#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereScatteringShader.h"
#include "platform/opengl/rendering/shaders/atmosphere/GLAtmosphereShader.h"
#include "platform/opengl/rendering/shaders/basic/GLBasicShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLNormalmapShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLSplatmapShader.h"
#include "platform/opengl/rendering/shaders/gpgpu/GLTerrainHeightsShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"
#include "platform/opengl/rendering/shaders/pbr/GLPBRShader.h"
#include "platform/opengl/rendering/shaders/terrain/GLTerrainShader.h"
#include "platform/opengl/rendering/shaders/terrain/GLTerrainWireframeShader.h"

#include "platform/vulkan/rendering/shaders/basic/VKBasicShader.h"
#include "platform/vulkan/rendering/shaders/pbr/VKPBRShader.h"

#define MODEL_PATH "res/models/"
#define TEXTURE_PATH "res/textures/"
#define SHADER_PATH "res/shaders/"

namespace Prehistoric
{
	/*
		This system works like this: every asset is stored here, and when we need one, we just pass in the the size_t ID of the asset, and we get a pointer back
	*/
	class AssetManager
	{
	public:
		AssetManager(Window* window);
		AssetManager(const AssetManager&) = default;
		~AssetManager() {}

		template<typename Resource>
		size_t getResource(const std::string& path)
		{
			throw std::string("size_t AssetManager::getResource(const std::string&) failed, recieved bad type: ") + typeid(Resource).name();
			return -1;
		}

		/*
			Use this function if we are sure that the resource exists
		*/
		template<typename Resource>
		size_t addResource(Resource* ptr)
		{
			throw std::string("size_t AssetManager::addResource(Resource*) failed, recieved bad type: ") + typeid(Resource).name();
			return -1;
		}

		/*
			Use this function if we are sure that the resource exists
		*/
		template<typename Resource>
		void addReference(size_t ID)
		{
			throw std::string("void AssetManager::addReference(size_t) failed, recieved bad type: ") + typeid(Resource).name();
		}

		template<typename Resource>
		void removeReference(size_t ID)
		{
			throw std::string("void AssetManager::removeReference(size_t) failed, recieved bad type: ") + typeid(Resource).name();
		}

		/*
			Returns the resource pointer (non-owner)
		*/
		template<typename Resource>
		Resource* getResourceByID(size_t ID)
		{
			throw std::string("Resource* AssetManager::getResourceByID(size_t) failed, recieved bad type: ") + typeid(Resource).name();
			return nullptr;
		}

		template<>
		size_t getResource<Texture>(const std::string& path)
		{
			auto index = ID_map.find(path);
			if (index != ID_map.end())
			{
				return index->second;
			}

			Texture* tex = TextureLoader::LoadTexture(/*TEXTURE_PATH + */path, window);
			size_t ret = texture_ID++;

			textures.insert(std::make_pair(ret, std::make_pair(tex, 0)));
			ID_map.insert(std::make_pair(path, ret));

			return ret;
		}

		template<>
		size_t getResource<VertexBuffer>(const std::string& path)
		{
			auto index = ID_map.find(path);
			if (index != ID_map.end())
			{
				return index->second;
			}

			VertexBuffer* buff = OBJLoader::LoadModel(MODEL_PATH, path, "", window);
			size_t ret = vertexBuffer_ID++;

			vertexBuffers.insert(std::make_pair(ret, std::make_pair(buff, 0)));
			ID_map.insert(std::make_pair(path, ret));

			return ret;
		}

		template<>
		size_t getResource<Shader>(const std::string& path)
		{
			auto index = ID_map.find(path);
			if (index != ID_map.end())
			{
				return index->second;
			}

			Shader* shader;
			//TODO: temporary solution:
			if (FrameworkConfig::api == OpenGL)
			{
				if (path == "pbr")
				{
					shader = new GLPBRShader();
				}
				else if (path == "basic")
				{
					shader = new GLBasicShader();
				}
				else if (path == "atmosphere_scattering")
				{
					shader = new GLAtmosphereScatteringShader();
				}
				else if (path == "atmosphere")
				{
					shader = new GLAtmosphereShader();
				}
				else if (path == "terrain_wireframe")
				{
					shader = new GLTerrainWireframeShader();
				}
				else if (path == "terrain")
				{
					shader = new GLTerrainShader();
				}
				else if (path == "gui")
				{
					shader = new GLGUIShader();
				}
				else if (path == "gpgpu_normal")
				{
					shader = new GLNormalMapShader();
				}
				else if (path == "gpgpu_splat")
				{
					shader = new GLSplatMapShader();
				}
				else if (path == "gpgpu_terrain_heights")
				{
					shader = new GLTerrainHeightsShader();
				}
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				if (path == "pbr")
				{
					shader = new VKPBRShader(window);
				}
				else if (path == "basic")
				{
					shader = new VKBasicShader(window);
				}
			}

			size_t ret = shader_ID++;

			shaders.insert(std::make_pair(ret, std::make_pair(shader, 0)));
			ID_map.insert(std::make_pair(path, ret));

			return ret;
		}

		template<>
		size_t addResource<Texture>(Texture* ptr)
		{
			for (auto& elem : textures)
			{
				if (elem.second.first.get() == ptr)
				{
					return elem.first;
				}
			}

			size_t ret = texture_ID++;
			textures.insert(std::make_pair(ret, std::make_pair(ptr, 0)));
			return ret;
		}

		template<>
		size_t addResource<VertexBuffer>(VertexBuffer* ptr)
		{
			for (auto& elem : vertexBuffers)
			{
				if (elem.second.first.get() == ptr)
				{
					return elem.first;
				}
			}

			size_t ret = vertexBuffer_ID++;
			vertexBuffers.insert(std::make_pair(ret, std::make_pair(ptr, 0)));
			return ret;
		}

		template<>
		size_t addResource<Shader>(Shader* ptr)
		{
			for (auto& elem : shaders)
			{
				if (elem.second.first.get() == ptr)
				{
					return elem.first;
				}
			}

			size_t ret = shader_ID++;
			shaders.insert(std::make_pair(ret, std::make_pair(ptr, 0)));
			return ret;
		}

		template<>
		void addReference<Texture>(size_t ID)
		{
			textures.at(ID).second++;
		}

		template<>
		void addReference<VertexBuffer>(size_t ID)
		{
			vertexBuffers.at(ID).second++;
		}

		template<>
		void addReference<Shader>(size_t ID)
		{
			shaders.at(ID).second++;
		}

		template<>
		void removeReference<Texture>(size_t ID)
		{
			uint32_t& refCount = textures.at(ID).second;
			refCount--;

			if (refCount == 0)
			{
				textures.erase(ID);

				for (const auto& elem : ID_map)
				{
					if (elem.second == ID)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		void removeReference<VertexBuffer>(size_t ID)
		{
			uint32_t& refCount = vertexBuffers.at(ID).second;
			refCount--;

			if (refCount == 0)
			{
				vertexBuffers.erase(ID);

				for (const auto& elem : ID_map)
				{
					if (elem.second == ID)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		void removeReference<Shader>(size_t ID)
		{
			uint32_t& refCount = shaders.at(ID).second;
			refCount--;

			if (refCount == 0)
			{
				shaders.erase(ID);

				for (const auto& elem : ID_map)
				{
					if (elem.second == ID)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		Texture* getResourceByID<Texture>(size_t ID)
		{
			return textures.at(ID).first.get();
		}

		template<>
		VertexBuffer* getResourceByID<VertexBuffer>(size_t ID)
		{
			return vertexBuffers.at(ID).first.get();
		}

		template<>
		Shader* getResourceByID<Shader>(size_t ID)
		{
			return shaders.at(ID).first.get();
		}

	private:
		Window* window;

		//size_t: ID, pointer: data, uint32_t: refcount
		std::unordered_map<size_t, std::pair<std::unique_ptr<Texture>, uint32_t>> textures;
		std::unordered_map<size_t, std::pair<std::unique_ptr<VertexBuffer>, uint32_t>> vertexBuffers;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Shader>, uint32_t>> shaders;

		//Maps for getting the resource ID from the name (only one because there can't be two things with the same name because of the system's naming conventions
		std::unordered_map<std::string, size_t> ID_map;

		//The IDs that we assign the next asset we add
		size_t texture_ID = 0;
		size_t vertexBuffer_ID = 0;
		size_t shader_ID = 0;

		//These values are just for reference, so I'll mark it as TODO
		constexpr static size_t TexturesSize = 30;
		constexpr static size_t VertexBuffersSize = 10;
		constexpr static size_t ShadersSize = 10;
	};
};

#endif