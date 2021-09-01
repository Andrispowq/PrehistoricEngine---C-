#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "Includes.hpp"

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"

#include "prehistoric/common/framework/Window.h"

#include "loaders/TextureLoader.h"
#include "loaders/VertexBufferLoader.h"
#include "loaders/ShaderLoader.h"

#define MODEL_PATH "res/models/"
#define TEXTURE_PATH "res/textures/"
#define SHADER_PATH "res/shaders/"

#define HANDLE_OF(type) typedef struct type##_handle_t { type* pointer = nullptr; size_t handle = -1; type* operator->() { return pointer;}; type* const operator->() const { return pointer;} } type##Handle

namespace Prehistoric
{
	HANDLE_OF(Texture);
	HANDLE_OF(VertexBuffer);
	HANDLE_OF(Shader);

	enum class BatchSettings
	{
		Load, QueuedLoad
	};

	enum class Shaders
	{
		BASIC,
		PBR
	};

	/*
		DOCUMENTATION of AssetManager

		This class stores every single resource we will ever use, loads them, stores them, and destroys them when needed.
		The client and the engine interacts with the resources through Handles, like this, for resource type T:

		struct THandle
		{
			T* pointer;
			size_t handle;
		};

		where 'pointer' is a pointer to the resource for ease of access, and 'handle' is the actual handle which helps with ref counting, creation and deletion.
		The handle is not a transparent reference, it's an opaque handle, which is never updated, but the system will unload resources only when there are no
		references left, this should not be a problem, since by reloading a resource, every reference to it will have decayed
	
		FUNCTIONS

		std::optional<THandle> loadT(BatchSettings, Args...) -> this function will load in one instance of T. Every resource requires some parameters, 
			like the texture or model's location,
			shader and material used in a pipeline, etc, this is what the Args... represents.
			The BatchSettings parameter is used for batching resource loading.

			enum class BatchSettings
			{
				Load, QueuedLoad
			};

			Load -> the resource must be loaded instantly, blocking the current thread (Synchronised)
			QueuedLoad -> the resource is added to the load queue, and will be dispatched soon (Asynchronous)
			
			The return type is an optional THandle struct, which will contain the loaded resource when using the Load option, will contain
			the resource that was queued for loading last when using the Dispatch mode, or will be a nullopt, when using the QueuedLoad mode

		std::vector<THandle>&& getLoadedHandles() -> this function returns all of the loaded resources of type T

		THandle storeT(T* t) -> this function stores one resource of type T in the registry, and returns the appropriate handle

		void addReference(size_t) -> this function increases the refcount of the resource associated with the handle passed in
		void removeReference(size_t) -> this function decreases the refcount of the resource associated with the handle passed in, and deletes it, if it reaches 0
	*/

	class AssetManager
	{
	public:
		AssetManager(Window* window);
		AssetManager(const AssetManager&) = default;
		~AssetManager() {}

		std::optional<TextureHandle> loadTexture(const std::string& location, SamplerFilter filter = Bilinear, TextureWrapMode wrapMode = ClampToEdge, BatchSettings settings = BatchSettings::Load);
		std::optional<VertexBufferHandle> loadVertexBuffer(std::optional<Model> mesh, const std::string& name, BatchSettings settings = BatchSettings::Load);
		std::optional<ShaderHandle> loadShader(ShaderName type, BatchSettings settings = BatchSettings::Load);

		TextureHandle storeTexture(Texture* texture, const std::string& cacheName = "");
		VertexBufferHandle storeVertexBuffer(VertexBuffer* vertexBuffer, const std::string& cacheName = "");
		ShaderHandle storeShader(Shader* shader);

		template<typename T> void addReference(size_t handle) {}
		template<typename T> void removeReference(size_t handle) {}
		template<typename T> std::vector<T*> get() {}

		template<>
		void addReference<Texture>(size_t handle)
		{
			auto& idx = textures.find(handle);
			if (idx == textures.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Texture handle %x\n", handle);
			}

			idx->second.second++;
		}

		template<>
		void addReference<VertexBuffer>(size_t handle)
		{
			auto& idx = vertexBuffers.find(handle);
			if (idx == vertexBuffers.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find VertexBuffer handle %x\n", handle);
			}

			idx->second.second++;
		}

		template<>
		void addReference<Shader>(size_t handle)
		{
			auto& idx = shaders.find(handle);
			if (idx == shaders.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Shader handle %x\n", handle);
			}

			idx->second.second++;
		}

		template<>
		void removeReference<Texture>(size_t handle)
		{
			auto& idx = textures.find(handle);
			if (idx == textures.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Texture handle %x\n", handle);
			}

			uint32_t& refCount = idx->second.second;
			refCount--;

			if (refCount == 0)
			{
				textures.erase(handle);

				for (const auto& elem : ID_map)
				{
					if (elem.second == handle)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		void removeReference<VertexBuffer>(size_t handle)
		{
			auto& idx = vertexBuffers.find(handle);
			if (idx == vertexBuffers.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find VertexBuffer handle %x\n", handle);
			}

			uint32_t& refCount = idx->second.second;
			refCount--;

			if (refCount == 0)
			{
				vertexBuffers.erase(handle);

				for (const auto& elem : ID_map)
				{
					if (elem.second == handle)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		void removeReference<Shader>(size_t handle)
		{
			auto& idx = shaders.find(handle);
			if (idx == shaders.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Shader handle %x\n", handle);
			}

			uint32_t& refCount = idx->second.second;
			refCount--;

			if (refCount == 0)
			{
				shaders.erase(handle);

				for (const auto& elem : ID_map)
				{
					if (elem.second == handle)
					{
						ID_map.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		std::vector<Texture*> get<Texture>()
		{
			std::vector<Texture*> ret;
			ret.reserve(textures.size());
			for (auto& _ : textures)
			{
				ret.push_back(_.second.first.get());
			}

			return ret;
		}

		template<>
		std::vector<VertexBuffer*> get<VertexBuffer>()
		{
			std::vector<VertexBuffer*> ret;
			ret.reserve(vertexBuffers.size());
			for (auto& _ : vertexBuffers)
			{
				ret.push_back(_.second.first.get());
			}

			return ret;
		}

		template<>
		std::vector<Shader*> get<Shader>()
		{
			std::vector<Shader*> ret;
			ret.reserve(shaders.size());
			for (auto& _ : shaders)
			{
				ret.push_back(_.second.first.get());
			}

			return ret;
		}

		TextureLoader* getTextureLoader() const { return textureLoader; }
		VertexBufferLoader* getVertexBufferLoader() const { return vertexBufferLoader; }
		ShaderLoader* getShaderLoader() const { return shaderLoader; }

	private:
		Window* window;

		//size_t: ID, pointer: data, uint32_t: refcount
		std::unordered_map<size_t, std::pair<std::unique_ptr<Texture>, uint32_t>> textures;
		std::unordered_map<size_t, std::pair<std::unique_ptr<VertexBuffer>, uint32_t>> vertexBuffers;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Shader>, uint32_t>> shaders;

		TextureLoader* textureLoader;
		VertexBufferLoader* vertexBufferLoader;
		ShaderLoader* shaderLoader;

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