#include "Includes.hpp"
#include "AssembledAssetManager.h"

namespace Prehistoric
{
	AssembledAssetManager::AssembledAssetManager(Window* window)
		: manager{nullptr}
	{
		this->window = window;

		manager = std::make_unique<AssetManager>(window);

		pipelines.reserve(PipelinesSize);
		materials.reserve(MaterialsSize);
	}

	PipelineHandle AssembledAssetManager::storePipeline(Pipeline* pipeline)
	{
		for (auto& entry : pipelines)
		{
			if (entry.second.first.get() == pipeline)
			{
				return { pipeline, entry.second.second };
			}
		}

		size_t han = pipeline_ID++;

		PipelineHandle handle;
		handle.pointer = pipeline;
		handle.handle = han;

		pipelines.insert(std::make_pair(handle.handle, std::make_pair(pipeline, 0)));

		return handle;
	}

	MaterialHandle AssembledAssetManager::storeMaterial(Material* material)
	{
		for (auto& entry : materials)
		{
			if (entry.second.first.get() == material)
			{
				return { material, entry.second.second };
			}
		}

		size_t han = material_ID++;

		MaterialHandle handle;
		handle.pointer = material;
		handle.handle = han;

		materials.insert(std::make_pair(handle.handle, std::make_pair(material, 0)));

		return handle;
	}
};