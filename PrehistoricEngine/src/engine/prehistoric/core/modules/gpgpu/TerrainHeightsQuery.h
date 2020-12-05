#ifndef TERRAIN_HEIGHTS_QUERY_H
#define TERRAIN_HEIGHTS_QUERY_H

#include "engine/platform/opengl/texture/GLTexture.h"
#include "engine/platform/vulkan/texture/VKTexture.h"

#include "engine/platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "engine/platform/vulkan/rendering/pipeline/VKComputePipeline.h"

#include "engine/platform/opengl/buffer/GLShaderStorageBuffer.h"
//#include "engine/platform/vulkan/buffer/GLShaderStorageBuffer.h"

#include "engine/platform/opengl/rendering/shaders/gpgpu/GLTerrainHeightsShader.h"
//#include "engine/platform/vulkan/rendering/shaders/gpgpu/VKerrainHeightsShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;

	class TerrainHeightsQuery
	{
	public:
		TerrainHeightsQuery(Window* window, AssembledAssetManager* manager, uint32_t N);
		virtual ~TerrainHeightsQuery();

		void Query(Texture* heightmap);

		float* getHeights() { return heights; }
	private:
		Window* window;
		AssembledAssetManager* manager;

		size_t pipelineID;
		Pipeline* pipeline;

		float* heights;
		ShaderStorageBuffer* buffer;

		uint32_t N;
	};
};

#endif