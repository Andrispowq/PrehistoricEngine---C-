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
		TerrainMaps(Window* window, AssembledAssetManager* manager, const std::string& terrainConfigFile);
		virtual ~TerrainMaps();

		Texture* getHeightmap() const { return heightmap.pointer; }
		Texture* getNormalmap() const { return normalmap.pointer; }
		Texture* getSplatmap() const { return splatmap.pointer; }

		float* getHeights() const { return heights; }

		std::vector<MaterialHandle> getMaterials() const { return materials; }
	private:
		void LoadConfigFile(const std::string& terrainConfigFile);
	private:
		Window* window;
		AssembledAssetManager* manager;

		TextureHandle heightmap;
		TextureHandle normalmap;
		TextureHandle splatmap;

		float* heights;

		NormalMapRenderer* normalmapRendererComponent;
		SplatMapRenderer* splatmapRendererComponent;
		TerrainHeightsQuery* query;

		std::string heightmapLocation;
		std::vector<MaterialHandle> materials;
		std::vector<std::pair<size_t, std::string>> textureLocations;
	};
};

#endif