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

		PipelineHandle storePipeline(Pipeline* pipeline);
		MaterialHandle storeMaterial(Material* material);

		template<typename T> void addReference(size_t handle) {}
		template<typename T> void removeReference(size_t handle) {}
		template<typename T> std::vector<T*> get() {}

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
		void removeReference<Pipeline>(size_t handle)
		{
			uint32_t& refCount = pipelines.at(handle).second;
			refCount--;

			if (refCount == 0)
			{
				pipelines.erase(handle);
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

		//The IDs that we assign the next asset we add
		size_t pipeline_ID = 0;
		size_t material_ID = 0;

		//These values are just for reference, so I'll mark it as TODO
		constexpr static size_t PipelinesSize = 40;
		constexpr static size_t MaterialsSize = 30;
	};
};

#endif