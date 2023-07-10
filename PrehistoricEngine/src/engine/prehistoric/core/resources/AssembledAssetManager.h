#ifndef ASSEMBLED_ASSET_MANAGER_H
#define ASSEMBLED_ASSET_MANAGER_H

#include "prehistoric/common/framework/Window.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"
#include "prehistoric/core/model/material/Material.h"

#include "AssetManager.h"

namespace Prehistoric
{
	HANDLE_OF(Pipeline);
	HANDLE_OF(Material);

	class AssembledAssetManager
	{
	public:
		AssembledAssetManager(Window* window);
		~AssembledAssetManager() {}

		PipelineHandle createPipeline(PipelineTypeHashFlags type, ShaderHandle shaderHandle, VertexBufferHandle vboHandle = { nullptr, (size_t)-1 });
		MaterialHandle getMaterial(size_t hash);

		PipelineHandle getPipelineFromHandle(size_t handle);
		MaterialHandle getMaterialFromHandle(size_t handle);

		PipelineHandle storePipeline(Pipeline* pipeline);
		MaterialHandle storeMaterial(Material* material);

		template<typename T> void addReference(size_t handle) {}
		template<typename T> void removeReference(size_t handle) {}
		template<typename T> std::vector<T*> get() {}

		template<>
		void addReference<Pipeline>(size_t handle)
		{
			auto& idx = pipelines.find(handle);
			if (idx == pipelines.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Pipeline handle %x\n", handle);
			}

			idx->second.second++;
		}

		template<>
		void addReference<Material>(size_t handle)
		{
			auto& idx = materials.find(handle);
			if (idx == materials.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Material handle %x\n", handle);
			}

			idx->second.second++;
		}
		
		template<>
		void removeReference<Pipeline>(size_t handle)
		{
			auto& idx = pipelines.find(handle);
			if (idx == pipelines.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Pipeline handle %x\n", handle);
			}

			uint32_t& refCount = idx->second.second;
			refCount--;

			if (refCount == 0)
			{
				pipelines.erase(handle);

				for (const auto& elem : pipelineHashMap)
				{
					if (elem.second == handle)
					{
						pipelineHashMap.erase(elem.first);
						return;
					}
				}
			}
		}

		template<>
		void removeReference<Material>(size_t handle)
		{
			auto& idx = materials.find(handle);
			if (idx == materials.end())
			{
				PR_LOG_RUNTIME_ERROR("Could not find Material handle %x\n", handle);
			}

			uint32_t& refCount = idx->second.second;
			refCount--;

			if (refCount == 0)
			{
				materials.erase(handle);

				for (const auto& elem : materialHashMap)
				{
					if (elem.second == handle)
					{
						materialHashMap.erase(elem.first);
						return;
					}
				}
			}
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

		AssetManager* getAssetManager() { return manager.get(); }

	private:
		Window* window;

		std::unique_ptr<AssetManager> manager;

		//size_t: ID, pointer: data, uint32_t: refcount
		std::unordered_map<size_t, std::pair<std::unique_ptr<Pipeline>, uint32_t>> pipelines;
		std::unordered_map<size_t, std::pair<std::unique_ptr<Material>, uint32_t>> materials;

		//You give it a hash and get back the corresponding ID
		std::unordered_map<size_t, size_t> pipelineHashMap;
		std::unordered_map<size_t, size_t> materialHashMap;

		//The IDs that we assign the next asset we add
		size_t pipeline_ID = 0;
		size_t material_ID = 0;

		//These values are just for reference, so I'll mark it as TODO
		constexpr static size_t PipelinesSize = 40;
		constexpr static size_t MaterialsSize = 30;
	};
};

#endif