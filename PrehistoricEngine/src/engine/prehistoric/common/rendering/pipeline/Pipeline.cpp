#include "Includes.hpp"
#include "Pipeline.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	Pipeline::Pipeline(Window* window, AssetManager* manager, ShaderHandle shader)
		: viewportStart{ 0, 0 }, viewportSize{ (float)__FrameworkConfig.windowWidth, (float)__FrameworkConfig.windowHeight },
		scissorStart{ 0, 0 }, scissorSize{ __FrameworkConfig.windowWidth, __FrameworkConfig.windowHeight }
	{
		this->window = window;
		this->manager = manager;

		this->shader = shader;
		manager->addReference<Shader>(shader.handle);
	}

	Pipeline::~Pipeline()
	{
		if(shader.pointer != nullptr)
			manager->removeReference<Shader>(shader.handle);
	}

	uint64_t Pipeline::GetHash(APIHashFlags apiFlags, PipelineTypeHashFlags typeFlags, size_t shaderHandle, size_t otherHandle)
	{
		uint64_t hash = 0;

		hash |= uint64_t(apiFlags) << 0;
		hash |= uint64_t(typeFlags) << 2;
		hash |= shaderHandle << 4;
		hash |= otherHandle << 32;

		return hash;
	}

	void Pipeline::SetHashInternal(APIHashFlags apiFlags, PipelineTypeHashFlags typeFlags, size_t shaderHandle, size_t otherHandle)
	{
		hash |= uint64_t(apiFlags) << 0;
		hash |= uint64_t(typeFlags) << 2;
		hash |= shaderHandle << 4;
		hash |= otherHandle << 32;
	}

	uint64_t Pipeline::GetHash()
	{
		if (hash)
			return hash;

		//The default settings
		SetHashInternal(APIHashFlags::GL, PipelineTypeHashFlags::Graphics, shader.handle, 0);
		return hash;
	}
};
