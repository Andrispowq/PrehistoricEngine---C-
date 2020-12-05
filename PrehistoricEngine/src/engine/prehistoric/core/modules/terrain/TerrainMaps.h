#ifndef TERRAIN_MAPS_H
#define TERRAIN_MAPS_H

#include "prehistoric/core/config/TerrainConfig.h"

#include "platform/opengl/texture/GLTexture.h"
#include "platform/vulkan/texture/VKTexture.h"

#include "prehistoric/core/modules/gpgpu/NormalMapRenderer.h"
#include "prehistoric/core/modules/gpgpu/SplatMapRenderer.h"
#include "prehistoric/core/modules/gpgpu/TerrainHeightsQuery.h"

namespace Prehistoric
{
	class TerrainMaps
	{
	public:
		TerrainMaps(Window* window, AssembledAssetManager* manager);
		virtual ~TerrainMaps();

		Texture* getHeightmap() const { return heightmap; }
		Texture* getNormalmap() const { return normalmap; }
		Texture* getSplatmap() const { return splatmap; }

		float* getHeights() const { return heights; }
	private:
		Window* window;
		AssembledAssetManager* manager;

		size_t heightmapID;

		Texture* heightmap;
		Texture* normalmap;
		Texture* splatmap;

		float* heights;

		NormalMapRenderer* normalmapRendererComponent;
		SplatMapRenderer* splatmapRendererComponent;
		TerrainHeightsQuery* query;
	};
};

#endif