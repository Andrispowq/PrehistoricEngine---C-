#ifndef ENVIROMENT_MAP_CONFIG_H
#define ENVIROMENT_MAP_CONFIG_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/common/texture/Texture.h"

namespace Prehistoric
{
    namespace _EnvironmentMapConfig
    {
        void LoadConfig(const std::string& path);

        extern std::string environmentMapLocation;
        extern uint32_t environmentMapResolution;
        extern uint32_t irradianceMapResolution;
        extern uint32_t prefilterMapResolution;
        extern uint32_t prefilterLevels;

        extern Texture* environmentMap;
        extern Texture* irradianceMap;
        extern Texture* prefilterMap;
        extern Texture* brdfLUT;
    };
};

#endif