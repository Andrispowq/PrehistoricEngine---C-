#ifndef TERRAIN_MAPS_H
#define TERRAIN_MAPS_H

#include "engine/config/TerrainConfig.h"

#include "engine/platform/opengl/texture/GLTexture.h"
#include "engine/platform/vulkan/texture/VKTexture.h"

#include "engine/prehistoric/modules/gpgpu/NormalMapRenderer.h"
#include "engine/prehistoric/modules/gpgpu/SplatMapRenderer.h"
#include "engine/prehistoric/modules/gpgpu/TerrainHeightsQuery.h"

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

#endif