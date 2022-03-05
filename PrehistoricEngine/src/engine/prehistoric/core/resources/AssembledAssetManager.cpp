#include "Includes.hpp"
#include "AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"

#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

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

	PipelineHandle AssembledAssetManager::createPipeline(PipelineTypeHashFlags type, ShaderHandle shaderHandle, VertexBufferHandle vboHandle)
	{
		uint64_t hash;
		if (__FrameworkConfig.api == OpenGL)
		{
			hash = Pipeline::GetHash(APIHashFlags::GL, type, shaderHandle.handle, vboHandle.handle);
		}
		else if (__FrameworkConfig.api == Vulkan)
		{
			hash = Pipeline::GetHash(APIHashFlags::VK, type, shaderHandle.handle, vboHandle.handle);
		}

		auto idx = pipelineHashMap.find(hash);
		if (idx != pipelineHashMap.end())
		{			
			return { pipelines[idx->second].first.get(), idx->second };
		}

		Pipeline* pipe = nullptr;

		switch (type)
		{
		case Prehistoric::PipelineTypeHashFlags::Graphics:
			if (__FrameworkConfig.api == OpenGL)
			{
				pipe = new GLGraphicsPipeline(window, manager.get(), shaderHandle, vboHandle);
			}
			else if(__FrameworkConfig.api == Vulkan)
			{
				pipe = new VKGraphicsPipeline(window, manager.get(), shaderHandle, vboHandle);
			}
			break;
		case Prehistoric::PipelineTypeHashFlags::Compute:
			if (__FrameworkConfig.api == OpenGL)
			{
				pipe = new GLComputePipeline(window, manager.get(), shaderHandle);
			}
			break;
		case Prehistoric::PipelineTypeHashFlags::RayTracing:
			break;
		default:
			break;
		}

		size_t han = pipeline_ID++;

		PipelineHandle handle;
		handle.pointer = pipe;
		handle.handle = han;

		pipelines.insert(std::make_pair(handle.handle, std::make_pair(pipe, 0)));
		pipelineHashMap.insert(std::make_pair(hash, handle.handle));

		return handle;
	}

	MaterialHandle AssembledAssetManager::getMaterial(size_t hash)
	{
		auto idx = materialHashMap.find(hash);
		if (idx != materialHashMap.end())
		{
			return { materials[idx->second].first.get(), idx->second };
		}

		return { nullptr, (size_t)-1 };
	}

	PipelineHandle AssembledAssetManager::storePipeline(Pipeline* pipeline)
	{
		auto idx = pipelineHashMap.find(pipeline->GetHash());
		if (idx != pipelineHashMap.end())
		{
			return { pipeline, idx->second };
		}

		size_t han = pipeline_ID++;

		PipelineHandle handle;
		handle.pointer = pipeline;
		handle.handle = han;

		pipelines.insert(std::make_pair(handle.handle, std::make_pair(pipeline, 0)));
		pipelineHashMap.insert(std::make_pair(pipeline->GetHash(), handle.handle));

		return handle;
	}

	MaterialHandle AssembledAssetManager::storeMaterial(Material* material)
	{
		uint64_t hash;
		if (material == nullptr)
			hash = 0;
		else
			hash = material->GetHash();

		auto idx = materialHashMap.find(hash);
		if (idx != materialHashMap.end())
		{
			return { material, idx->second };
		}

		size_t han = material_ID++;

		MaterialHandle handle;
		handle.pointer = material;
		handle.handle = han;

		materials.insert(std::make_pair(handle.handle, std::make_pair(material, 0)));
		materialHashMap.insert(std::make_pair(hash, handle.handle));

		return handle;
	}
};