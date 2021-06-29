#include "Includes.hpp"
#include "TerrainHeightsQuery.h"

namespace Prehistoric
{
	TerrainHeightsQuery::TerrainHeightsQuery(Window* window, AssembledAssetManager* manager, uint32_t N)
	{
		this->window = window;
		this->manager = manager;

		this->N = N;

		AssetManager* man = manager->getAssetManager();

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = manager->storePipeline(new GLComputePipeline(window, man, man->loadShader(ShaderName::GPGPUHeightQuery).value()));
			manager->addReference<Pipeline>(pipeline.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = manager->storePipeline(new VKComputePipeline(window, man, man->loadShader("gpgpu_terrain_heights").value()));
			//manager->addReference<Pipeline>(pipeline.handle);
		}

		heights = new float[N * N];

		if (FrameworkConfig::api == OpenGL)
		{
			buffer = new GLShaderStorageBuffer(window, heights, N * N * sizeof(float));
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
		manager->removeReference<Pipeline>(pipeline.handle);
		delete buffer;
		delete[] heights;
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

		buffer->Bind(nullptr);
		buffer->MapBuffer();
		memcpy(heights, buffer->getMappedData(), N * N * sizeof(float));
		buffer->UnmapBuffer();
		buffer->Unbind();
	}
};
