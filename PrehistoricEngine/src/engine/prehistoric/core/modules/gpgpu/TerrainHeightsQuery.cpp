#include "Includes.hpp"
#include "TerrainHeightsQuery.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	TerrainHeightsQuery::TerrainHeightsQuery(Window* window, ResourceStorage* resourceStorage, uint32_t N)
	{
		this->window = window;
		this->resourceStorage = resourceStorage;

		this->N = N;

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = resourceStorage->storePipeline(new GLComputePipeline(window, resourceStorage, resourceStorage->loadShader("gpgpu_terrain_heights").value()));
			resourceStorage->addReference<Pipeline>(pipeline.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = resourceStorage->storePipeline(new VKComputePipeline(window, resourceStorage, resourceStorage->loadShader("gpgpu_terrain_heights").value()));
			//resourceStorage->addReference<Pipeline>(pipeline.handle);
		}

		heights = new float[N * N];

		Layout layout;
		layout.addLayoutMember(LayoutType::FLOAT, LayoutTypeInfo::UNSIZED_ARRAY, N * N);

		if (FrameworkConfig::api == OpenGL)
		{
			buffer = new GLShaderStorageBuffer(heights, layout);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//buffer = new VKShaderStorageBuffer();
		}

		if (FrameworkConfig::api == OpenGL)
		{
			GLComputePipeline* glPipe = reinterpret_cast<GLComputePipeline*>(pipeline.pointer);
			glPipe->setInvocationSize({ N / 16, N / 16, 1 });
			glPipe->addSSBOBinding(0, buffer, WRITE_ONLY);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			/*VKComputePipeline* vkPipe = reinterpret_cast<VKComputePipeline*>(pipeline);
			vkPipe->setInvocationSize({ float(N / 16), float(N / 16), 1.0f });
			vkPipe->addTextureBinding(0, buffer, WRITE_ONLY);*/
		}
	}

	TerrainHeightsQuery::~TerrainHeightsQuery()
	{
		resourceStorage->removeReference<Pipeline>(pipeline.handle);
		delete buffer;
	}

	void TerrainHeightsQuery::Query(Texture* heightmap)
	{
		pipeline->BindPipeline(nullptr);

		if (FrameworkConfig::api == OpenGL)
		{
			reinterpret_cast<GLTerrainHeightsShader*>(pipeline->getShader())->UpdateUniforms(heightmap, N);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//reinterpret_cast<VKTerrainHeightsShader*>(pipeline->getShader())->UpdateUnifroms(heightmap, N);
		}

		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();

		buffer->Bind(nullptr, 0);
		buffer->MapBuffer();
		memcpy(heights, buffer->getMappedData(), N * N * sizeof(float));
		buffer->UnmapBuffer();
		buffer->Unbind();
	}
};
