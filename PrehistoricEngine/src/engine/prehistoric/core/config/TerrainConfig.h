#ifndef TERRAIN_CONFIG_H
#define TERRAIN_CONFIG_H

#include "Includes.hpp"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
	class Material;

	class Window;
	class AssetManager;

	namespace TerrainConfig
	{
		void LoadConfig(const std::string& path);
		int UpdateMorphingArea(int lod);

		extern float scaleY;
		extern float scaleXZ;

		extern int tessellationFactor;
		extern float tessellationSlope;
		extern float tessellationShift;

		extern std::vector<int> lodRanges;
		extern std::vector<int> lodMorphingAreas;

		extern std::vector<Material*> terrainMaterials;

		extern Texture* heightmap;
		extern Texture* normalmap;
		extern Texture* splatmap;
	};
};

#endif