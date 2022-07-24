#ifndef ENGINE_SETTINGS_H
#define ENGINE_SETTINGS_H

#include <fstream>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/common/texture/Texture.h"

#include "prehistoric/core/modules/terrain/TerrainQuadtree.h"
#include "prehistoric/core/modules/water/WaterQuadtree.h"
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

    struct WaterConfig
    {
        int UpdateMorphingArea(int lod)
        {
            return (int)((scaleXZ / WaterQuadtree::rootNodes) / pow(2, lod));
        }

        float scaleXZ = 0.0f;

        int tessellationFactor = 0;
        float tessellationSlope = 0.0f;
        float tessellationShift = 0.0f;

        std::vector<int> lodRanges = {};
        std::vector<int> lodMorphingAreas = {};

        Vector3f baseColour = 0;
        float displacementScale = 0.0f;
        int displacementRange = 0;
        int tiling = 0;
        float choppiness = 0.0f;
        float wavemotion = 0.0f;
        float normalStrength = 0.0f;
        int highDetailRange = 0;
        float capillarStrength = 0.0f;
        float capillarDownsampling = 0.0f;

        float fresnelFactor = 0.0f;
        float kReflection = 0.0f;
        float kRefraction = 0.0f;
        float reflectionBlendfactor = 0.0f;
        float dudvDownsampling = 0.0f;
        float underwaterBlurfactor = 0.0f;
        float distortionDelta = 0.0f;

        float specularFactor = 0.0f;
        float specularAmplifier = 0.0f;
        float emissionFactor = 0.0f;
        bool diffuse = false;

        Vector2f windDirection = 0;
        float windSpeed = 0.0f;
        float alignment = 0.0f;
        int fftResolution = 0;
        float fftAmplitude = 0.0f;
        int fftL = 0;
        float fftCapillarWavesSupression = 0.0f;
        bool choppy = false;
        float tDelta = 0.0f;
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
        WaterConfig waterConfig;
    };
};

#endif