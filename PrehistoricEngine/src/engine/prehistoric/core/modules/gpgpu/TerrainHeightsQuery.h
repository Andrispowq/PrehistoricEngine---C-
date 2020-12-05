#ifndef TERRAIN_HEIGHTS_QUERY_H
#define TERRAIN_HEIGHTS_QUERY_H

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
//#include "platform/vulkan/rendering/pipeline/VKComputePipeline.h"

#include "platform/opengl/buffer/GLShaderStorageBuffer.h"
//#include "platform/vulkan/buffer/GLShaderStorageBuffer.h"

#include "platform/opengl/rendering/shaders/gpgpu/GLTerrainHeightsShader.h"
//#include "platform/vulkan/rendering/shaders/gpgpu/VKerrainHeightsShader.h"

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