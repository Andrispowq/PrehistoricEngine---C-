#ifndef RESOURCE_STORAGE_H
#define RESOURCE_STORAGE_H

#include "prehistoric/common/framework/Window.h"

#include "prehistoric/common/texture/Texture.h"
#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/shaders/Shader.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "loaders/TextureLoader.h"
#include "loaders/VertexBufferLoader.h"
#include "loaders/ShaderLoader.h"

namespace Prehistoric
{
	/*
	
		DOCUMENTATION of ResourceStorage

		This class stores every single resource we will ever use, loads them, stores them, and destroys them when needed.
		The client and the engine interacts with the resources through Handles, like this, for resource type T:

		struct THandle
		{
			T* pointer;
			size_t handle;
		};

		where 'pointer' is a pointer to the resource for ease of access, and 'handle' is the actual handle which helps with ref counting, creation and deletion.
		The handle is not a transparent reference, it's an opaque handle, which is never updated, but the system will unload resources only when there are no
		references left, this should not be a problem, since by reloading a resource, every reference to it will have been decayed
	
		FUNCTIONS

		std::optional<THandle> loadT(BatchSettings, Args...) -> this function will load in one instance of T. Every resource requires some parameters, 
			like the texture or model's location,
			shader and material used in a pipeline, etc, this is what the Args... represents.
			The BatchSettings parameter is used for batching resource loading.

			enum class BatchSettings
			{
				Load, QueuedLoad, Dispatch
			};

			Load -> the resource must be loaded instantly, blocking the current thread (Synchronised)
			QueuedLoad -> the resource is added to the load queue, and will be dispatched soon (Asynchronous)
			Dispatch -> every queued load operation for that type of resource must be dispatched immidietly (but it could've been started), blocking the current thread, until it's finished (Asynchronous)

			The return type is an optional THandle struct, which will contain the loaded resource when using the Load option, will contain
			the resource that was queued for loading last when using the Dispatch mode, or will be a nullopt, when using the QueuedLoad mode

		std::vector<THandle>&& getLoadedHandles() -> this function returns all of the loaded resources of type T, except for the last one, which is returned on loadT

		THandle storeT(T* t) -> this function stores one resource of type T in the registry, and returns the appropriate handle

		void addReference(size_t) -> this function increases the refcount of the resource associated with the handle passed in
		void removeReference(size_t) -> this function decreases the refcount of the resource associated with the handle passed in, and deletes it, if it reaches 0

	*/

	/*
	
		NOTE:

		The handles of the data stored here are not actually defined here, rather in their header files respecively
		This is because there was a header circular dependency issue, and this is the current solution
		In the future it might get put back here which is the most convinient place for them
	
	*/

	enum class BatchSettings
	{
		Load, QueuedLoad, Dispatch
	};

	enum class Shaders
	{
		BASIC,
		PBR
	};

	class ResourceStorage
	{
	public:
		ResourceStorage(Window* window);
		virtual ~ResourceStorage() {}

		std::optional<TextureHandle> loadTexture(const std::string& location, SamplerFilter filter = Bilinear, TextureWrapMode wrapMode = ClampToEdge, BatchSettings settings = BatchSettings::Load);
		std::optional<VertexBufferHandle> loadVertexBuffer(std::optional<Mesh> mesh, const std::string& name, BatchSettings settings = BatchSettings::Load);
		std::optional<ShaderHandle> loadShader(const std::string& location, BatchSettings settings = BatchSettings::Load);
		
		TextureHandle storeTexture(Texture* texture);
		VertexBufferHandle storeVertexBuffer(VertexBuffer* vertexBuffer);
		ShaderHandle storeShader(Shader* shader);
		PipelineHandle storePipeline(Pipeline* pipeline);
		MaterialHandle storeMaterial(Material* material);

		template<typename T> void addReference(size_t handle) {}
		template<typename T> void removeReference(size_t handle) {}
		template<typename T> std::vector<T*> get() {}

		template<>
		void addReference<Texture>(size_t handle)
		{
			textures.at(handle).second++;
		}

		template<>
		void addReference<VertexBuffer>(size_t handle)
		{
			vertexBuffers.at(handle).second++;
		}

		template<>
		void addReference<Shader>(size_t handle)
		{
			shaders.at(handle).second++;
		}

		template<>
		void addReference<Pipeline>(size_t handle)
		{
			pipelines.at(handle).second++;
		}

		template<>
		void addReference<Material>(size_t handle)
		{
			materials.at(handle).second++;
		}

		template<>
		void removeReference<Texture>(size_t handle)
		{
			uint32_t& refCount = textures.at(handle).second;
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
			uint32_t& refCount = vertexBuffers.at(handle).second;
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
			uint32_t& refCount = shaders.at(handle).second;
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
		void removeReference<Pipeline>(size_t handle)
		{
			uint32_t& refCount = pipelines.at(handle).second;
			refCount--;

			if (refCount == 0)
			{
				pipelines.erase(handle);

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
		void removeReference<Material>(size_t handle)
		{
			uint32_t& refCount = materials.at(handle).second;
			refCount--;

			if (refCount == 0)
			{
				materials.erase(handle);

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

			return ret;;
		}

		template<>
		std::vector<Pipeline*> get<Pipeline>()
		{
			std::vector<Pipeline*> ret;
			ret.reserve(pipelines.size());
			for (auto& _ : pipelines)
			{
				ret.push_back(_.second.first.get());
			}

			return ret;
		}

		template<>
		std::vector<Material*> get<Material>()
		{
			std::vector<Material*> ret;
			ret.reserve(materials.size());
			for (auto& _ : materials)
			{
				ret.push_back(_.second.first.get());
			}

			return ret;
		}

		ResourceStorage(const ResourceStorage&) = default;
		ResourceStorage(ResourceStorage&&) = default;
		ResourceStorage& operator=(const ResourceStorage&) = default;
	private:
		Window* window;

		//size_t: ID, pointer: data, uint32_t: refcount
		std::unordered_map<size_t, std::pair<std::unique_ptr<Texture>, uint32_t>> textures;
		std::unordered_map<size_t, std::pair<std::unique_ptr<VertexBuffer>, uint32_t>> vertexBuffers;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Shader>, uint32_t>> shaders;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Pipeline>, uint32_t>> pipelines;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Material>, uint32_t>> materials;

		TextureLoader* textureLoader;
		VertexBufferLoader* vertexBufferLoader;
		ShaderLoader* shaderLoader;

		//This is used for caching resources that are used, so that requesting a model two times won't result in two loads
		std::unordered_map<std::string, size_t> ID_map;

		//The IDs that we assign the next asset we add
		size_t texture_ID = 0;
		size_t vertexBuffer_ID = 0;
		size_t shader_ID = 0;
		size_t pipeline_ID = 0;
		size_t material_ID = 0;

		//These values are just for reference, so I'll mark it as TODO
		constexpr static size_t TexturesSize = 100;
		constexpr static size_t VertexBuffersSize = 30;
		constexpr static size_t ShadersSize = 20;
		constexpr static size_t PipelinesSize = 100;
		constexpr static size_t MaterialsSize = 40;
	};
};

#endif