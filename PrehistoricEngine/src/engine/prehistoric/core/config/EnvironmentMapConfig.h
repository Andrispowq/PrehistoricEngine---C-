#ifndef ENVIROMENT_MAP_CONFIG_H
#define ENVIROMENT_MAP_CONFIG_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
    namespace EnvironmentMapConfig
    {
        PR_API void LoadConfig(const std::string& path);

        extern PR_API std::string environmentMapLocation;
        extern PR_API uint32_t environmentMapResolution;
        extern PR_API uint32_t irradianceMapResolution;
        extern PR_API uint32_t prefilterMapResolution;
        extern PR_API uint32_t prefilterLevels;

        extern PR_API Texture* environmentMap;
        extern PR_API Texture* irradianceMap;
        extern PR_API Texture* prefilterMap;
        extern PR_API Texture* brdfLUT;
    };
};

#endif