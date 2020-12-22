#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
    namespace EngineConfig
    {
        PR_API void LoadConfig(const std::string& path);

        extern PR_API uint32_t lightsMaxNumber;
        extern PR_API float rendererGamma;
        extern PR_API float rendererExposure;
        extern PR_API int rendererHighDetailRange;
        extern PR_API float rendererNearPlane;
        extern PR_API float rendererFarPlane;
    };
};

#endif