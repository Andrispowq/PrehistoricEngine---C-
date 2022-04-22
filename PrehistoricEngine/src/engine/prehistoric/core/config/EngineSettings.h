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
        std::string windowName = "";
        uint32_t windowWidth = 0;
        uint32_t windowHeight = 0;
        bool windowFullScreen = false;
        bool windowResizable = false;
        bool windowVSync = false;
        uint32_t windowNumSamples = 1;
        uint32_t windowMaxFPS = 60;

        std::map<API, Vector2i> apiVersions = {};

        API api = API::OpenGL;
        Vector2i apiVersion = Vector2i(1, 0);
        bool apiVulkanUseValidationLayers = false;
    };

    struct EngineConfig
    {
        uint32_t lightsMaxNumber = 10;
        float rendererGamma = 2.2f;
        float rendererExposure = 1.0f;
        int rendererHighDetailRange = 100;
        float rendererNearPlane = 0.1f;
        float rendererFarPlane = 100;
    };

    struct EnvironmentMapConfig
    {
        std::string environmentMapLocation = "";
        uint32_t environmentMapResolution = 0;
        uint32_t irradianceMapResolution = 0;
        uint32_t prefilterMapResolution = 0;
        uint32_t prefilterLevels = 0;

        Texture* environmentMap = nullptr;
        Texture* irradianceMap = nullptr;
        Texture* prefilterMap = nullptr;
        Texture* brdfLUT = nullptr;
    };

    struct AtmosphereConfig
    {
        Vector3f sunPosition = 0;
        float sunRadius = 0.0f;
        Vector3f sunColour = 0;
        float sunIntensity = 0.0f;

        Vector3f fogColour = 0;
        float fogBrightness = 0.0f;

        float planetRadius = 0.0f;
        float atmosphereRadius = 0.0f;

        Vector3f rayleigh = 0;
        float rayleighHeightScale = 0.0f;
        float mie = 0.0f;
        float mieHeightScale = 0.0f;
        float mieDirection = 0.0f;

        float sightRange = 0.0f;
        float ambient = 0.0f;
        float horizontalVerticalShift = 0.0f;
        float bloomFactor = 0.0f;
        bool scatteringEnabled = false;
    };

    struct TerrainConfig
    {
        int UpdateMorphingArea(int lod)
        {
            return (int)((scaleXZ / TerrainQuadtree::rootNodes) / pow(2, lod));
        }

        float scaleY = 0.0f;
        float scaleXZ = 0.0f;

        int tessellationFactor = 0;
        float tessellationSlope = 0.0f;
        float tessellationShift = 0.0f;

        std::vector<int> lodRanges = {};
        std::vector<int> lodMorphingAreas = {};
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