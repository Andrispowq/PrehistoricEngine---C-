#ifndef ENGINE_SETTINGS_H
#define ENGINE_SETTINGS_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/common/texture/Texture.h"

#include "prehistoric/core/modules/terrain/TerrainQuadtree.h"
#include "prehistoric/core/model/material/Material.h" 

namespace Prehistoric
{
    /*
        Implemented:
            -
        Under development:
            OpenGL, Vulkan
        Planned:
            Vulkan_RTX, DirectX_11, DirectX_12, DirectX_DXR
    */
    enum API
    {
        OpenGL, Vulkan, Vulkan_RTX, DirectX_11, DirectX_12, DirectX_DXR, NO_API
    };

    struct FrameworkConfig
    {
        std::string windowName;
        uint32_t windowWidth;
        uint32_t windowHeight;
        bool windowFullScreen;
        bool windowResizable;
        bool windowVSync;
        uint32_t windowNumSamples;
        uint32_t windowMaxFPS;

        std::map<API, Vector2i> apiVersions;

        API api;
        Vector2i apiVersion;
        bool apiVulkanUseValidationLayers;
    };

    struct EngineConfig
    {
        uint32_t lightsMaxNumber;
        float rendererGamma;
        float rendererExposure;
        int rendererHighDetailRange;
        float rendererNearPlane;
        float rendererFarPlane;
    };

    struct EnvironmentMapConfig
    {
        std::string environmentMapLocation;
        uint32_t environmentMapResolution;
        uint32_t irradianceMapResolution;
        uint32_t prefilterMapResolution;
        uint32_t prefilterLevels;

        Texture* environmentMap;
        Texture* irradianceMap;
        Texture* prefilterMap;
        Texture* brdfLUT;
    };

    struct AtmosphereConfig
    {
        Vector3f sunPosition;
        float sunRadius;
        Vector3f sunColour;
        float sunIntensity;

        Vector3f fogColour;
        float fogBrightness;

        float planetRadius;
        float atmosphereRadius;

        Vector3f rayleigh;
        float rayleighHeightScale;
        float mie;
        float mieHeightScale;
        float mieDirection;

        float sightRange;
        float ambient;
        float horizontalVerticalShift;
        float bloomFactor;
        bool scatteringEnabled;
    };

    struct TerrainConfig
    {
        int UpdateMorphingArea(int lod)
        {
            return (int)((scaleXZ / TerrainQuadtree::rootNodes) / pow(2, lod));
        }

        float scaleY;
        float scaleXZ;

        int tessellationFactor;
        float tessellationSlope;
        float tessellationShift;

        std::vector<int> lodRanges;
        std::vector<int> lodMorphingAreas;

        std::vector<Material*> terrainMaterials;

        Texture* heightmap;
        Texture* normalmap;
        Texture* splatmap;
    };

    class EngineSettings
    {
    public:
        void LoadConfig(const std::string& path);

    public:
        FrameworkConfig frameworkConfig;
        EngineConfig engineConfig;
        EnvironmentMapConfig environmentMapConfig;
        AtmosphereConfig atmosphereConfig;
        TerrainConfig terrainConfig;
    };
};

#endif