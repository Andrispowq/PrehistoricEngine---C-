#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <fstream>

#include "engine/prehistoric/core/util/Util.h"
#include "engine/prehistoric/core/util/math/Math.h"

namespace EngineConfig
{
    void LoadConfig(const std::string& path);

    extern uint32_t lightsMaxNumber;
    extern float rendererGamma;
    extern float rendererExposure;
    extern int rendererHighDetailRange;
    extern float rendererNearPlane;
    extern float rendererFarPlane;
};

#endif