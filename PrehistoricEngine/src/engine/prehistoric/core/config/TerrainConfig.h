#ifndef TERRAIN_CONFIG_H
#define TERRAIN_CONFIG_H

#include "Includes.hpp"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class Material;

	class Window;
	class AssetManager;

	namespace TerrainConfig
	{
		PR_API void LoadConfig(const std::string& path);
		PR_API int UpdateMorphingArea(int lod);

		extern PR_API float scaleY;
		extern PR_API float scaleXZ;

		extern PR_API int tessellationFactor;
		extern PR_API float tessellationSlope;
		extern PR_API float tessellationShift;

		extern PR_API std::vector<int> lodRanges;
		extern PR_API std::vector<int> lodMorphingAreas;
	};
};

#endif