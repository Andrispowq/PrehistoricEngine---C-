#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
    namespace _EngineConfig
    {
        void LoadConfig(const std::string& path);

        extern uint32_t lightsMaxNumber;
        extern float rendererGamma;
        extern float rendererExposure;
        extern int rendererHighDetailRange;
        extern float rendererNearPlane;
        extern float rendererFarPlane;
    };
};

#endif