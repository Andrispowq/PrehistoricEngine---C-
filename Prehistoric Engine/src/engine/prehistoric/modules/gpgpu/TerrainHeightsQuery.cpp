#include "engine/prehistoric/core/util/Includes.hpp"
#include "TerrainHeightsQuery.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

TerrainHeightsQuery::TerrainHeightsQuery(Window* window, AssembledAssetManager* manager, uint32_t N)
{
	this->window = window;
	this->manager = manager;
	
	this->N = N;

	//TODO: Create the Vulkan equivalent of the GLComputePipeline
	if (FrameworkConfig::api == OpenGL)
	{
		pipeline = new GLComputePipeline(window, manager->getAssetManager(), manager->getAssetManager()->getResource<Shader>("gpgpu_terrain_heights"));
		pipelineID = manager->loadResource<Pipeline>(pipeline);
		manager->addReference<Pipeline>(pipelineID);
	}
	else if (FrameworkConfig::api == Vulkan)
	{
		//pipeline = new VKComputePipeline(new VKTerrainHeightsShader());
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
		GLComputePipeline* glPipe = reinterpret_cast<GLComputePipeline*>(pipeline);
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
	manager->removeReference<Pipeline>(pipelineID);
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
