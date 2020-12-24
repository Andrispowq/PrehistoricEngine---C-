#ifndef ATMOSPHERE_CONFIG_H
#define ATMOSPHERE_CONFIG_H

#include "Includes.hpp"

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	namespace AtmosphereConfig
	{
		void LoadConfig(const std::string& path);

		extern Vector3f sunPosition;
		extern float sunRadius;
		extern Vector3f sunColour;
		extern float sunIntensity;

		extern Vector3f fogColour;
		extern float fogBrightness;

		extern float planetRadius;
		extern float atmosphereRadius;

		extern Vector3f rayleigh;
		extern float rayleighHeightScale;
		extern float mie;
		extern float mieHeightScale;
		extern float mieDirection;

		extern float sightRange;
		extern float ambient;
		extern float horizontalVerticalShift;
		extern float bloomFactor;
		extern bool scatteringEnabled;
	};
};

#endif