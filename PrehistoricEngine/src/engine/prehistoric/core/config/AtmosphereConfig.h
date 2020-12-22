#ifndef ATMOSPHERE_CONFIG_H
#define ATMOSPHERE_CONFIG_H

#include "Includes.hpp"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	namespace AtmosphereConfig
	{
		PR_API void LoadConfig(const std::string& path);

		extern PR_API Vector3f sunPosition;
		extern PR_API float sunRadius;
		extern PR_API Vector3f sunColour;
		extern PR_API float sunIntensity;

		extern PR_API Vector3f fogColour;
		extern PR_API float fogBrightness;

		extern PR_API float planetRadius;
		extern PR_API float atmosphereRadius;

		extern PR_API Vector3f rayleigh;
		extern PR_API float rayleighHeightScale;
		extern PR_API float mie;
		extern PR_API float mieHeightScale;
		extern PR_API float mieDirection;

		extern PR_API float sightRange;
		extern PR_API float ambient;
		extern PR_API float horizontalVerticalShift;
		extern PR_API float bloomFactor;
		extern PR_API bool scatteringEnabled;
	};
};

#endif