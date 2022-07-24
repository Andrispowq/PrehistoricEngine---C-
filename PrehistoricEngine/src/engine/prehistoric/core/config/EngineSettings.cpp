#include "Includes.hpp"
#include "EngineSettings.h"

#include <json.hpp>

namespace Prehistoric
{
	void EngineSettings::LoadConfig(const std::string& path)
	{
		std::ifstream file;
		file.open(path.c_str(), std::ios::ate);

		if (file.is_open())
		{
			std::string contents;
			size_t size = file.tellg();
			contents.resize(size);
			file.seekg(0);
			file.read(&contents[0], size);

			nlohmann::json file_json = nlohmann::json::parse(contents);

			{
				nlohmann::json framework_json = file_json["framework"];

				nlohmann::json window_json = framework_json["window"];
				frameworkConfig.windowName = window_json["name"];
				frameworkConfig.windowWidth = window_json["width"];
				frameworkConfig.windowHeight = window_json["height"];
				frameworkConfig.windowFullScreen = window_json["fullScreen"];
				frameworkConfig.windowResizable = window_json["resizable"];
				frameworkConfig.windowVSync = window_json["vSync"];
				frameworkConfig.windowNumSamples = window_json["numSamples"];
				frameworkConfig.windowMaxFPS = window_json["maxFPS"];

				nlohmann::json api_json = framework_json["api"];
				std::string activeAPI = api_json["active"];
				if (activeAPI == "OpenGL") frameworkConfig.api = API::OpenGL;
				else if (activeAPI == "Vulkan") frameworkConfig.api = API::Vulkan;

				nlohmann::json opengl_json = api_json["OpenGL"];
				frameworkConfig.apiVersions.insert(std::make_pair(API::OpenGL, opengl_json["version"]));

				nlohmann::json vulkan_json = api_json["Vulkan"];
				frameworkConfig.apiVersions.insert(std::make_pair(API::Vulkan, vulkan_json["version"]));
				frameworkConfig.apiVulkanUseValidationLayers = vulkan_json["useValidationLayers"];

				frameworkConfig.apiVersion = frameworkConfig.apiVersions[frameworkConfig.api];
			}

			{
				nlohmann::json engine_json = file_json["engine"];
				engineConfig.lightsMaxNumber = engine_json["maxNumberOfLights"];

				nlohmann::json renderer_json = engine_json["renderer"];
				engineConfig.rendererGamma = renderer_json["gamma"];
				engineConfig.rendererExposure = renderer_json["exposure"];
				engineConfig.rendererHighDetailRange = renderer_json["highDetailRange"];
				engineConfig.rendererNearPlane = renderer_json["nearPlane"];
				engineConfig.rendererFarPlane = renderer_json["farPlane"];
			}

			{
				nlohmann::json atmosphere_json = file_json["atmosphere"];
				
				nlohmann::json sun_json = atmosphere_json["sun"];
				std::vector<float> sun_position = sun_json["position"];
				atmosphereConfig.sunPosition = Vector3f(sun_position[0], sun_position[1], sun_position[2]);
				atmosphereConfig.sunRadius = sun_json["radius"];
				std::vector<float> sun_colour = sun_json["colour"];
				atmosphereConfig.sunColour = Vector3f(sun_colour[0], sun_colour[1], sun_colour[2]);
				atmosphereConfig.sunIntensity = sun_json["intensity"];

				nlohmann::json fog_json = atmosphere_json["fog"];
				std::vector<float> fog_colour = fog_json["colour"];
				atmosphereConfig.fogColour = Vector3f(fog_colour[0], fog_colour[1], fog_colour[2]);
				atmosphereConfig.fogBrightness = fog_json["brightness"];

				atmosphereConfig.planetRadius = atmosphere_json["planetRadius"];
				atmosphereConfig.atmosphereRadius = atmosphere_json["atmosphereRadius"];

				nlohmann::json coefficients_json = atmosphere_json["coefficients"];
				std::vector<float> rayleigh_vals = coefficients_json["rayleigh"];
				atmosphereConfig.rayleigh = Vector3f(rayleigh_vals[0], rayleigh_vals[1], rayleigh_vals[2]);
				atmosphereConfig.rayleighHeightScale = coefficients_json["rayleighHeightScale"];
				atmosphereConfig.mie = coefficients_json["mie"];
				atmosphereConfig.mieHeightScale = coefficients_json["mieHeightScale"];
				atmosphereConfig.mieDirection = coefficients_json["mieDirection"];

				atmosphereConfig.sightRange = atmosphere_json["sightRange"];
				atmosphereConfig.ambient = atmosphere_json["ambient"];
				atmosphereConfig.horizontalVerticalShift = atmosphere_json["horizontalVerticalShift"];
				atmosphereConfig.bloomFactor = atmosphere_json["bloomFactor"];
				atmosphereConfig.scatteringEnabled = atmosphere_json["scatteringEnabled"];
			}

			{
				nlohmann::json environment_map_json = file_json["environmentMap"];
				environmentMapConfig.environmentMapLocation = environment_map_json["location"];
				environmentMapConfig.environmentMapResolution = environment_map_json["environmentMapResolution"];
				environmentMapConfig.irradianceMapResolution = environment_map_json["irradianceMapResolution"];
				environmentMapConfig.prefilterMapResolution = environment_map_json["prefilterMapResolution"];
				environmentMapConfig.prefilterLevels = environment_map_json["prefilterMapLevels"];
			}

			{
				nlohmann::json terrain_json = file_json["terrain"];
				std::vector<float> scales = terrain_json["scale"];
				terrainConfig.scaleXZ = scales[0];
				terrainConfig.scaleY = scales[1];

				nlohmann::json tessellation_json = terrain_json["tessellation"];
				terrainConfig.tessellationFactor = tessellation_json["factor"];
				terrainConfig.tessellationSlope = tessellation_json["slope"];
				terrainConfig.tessellationShift = tessellation_json["shift"];

				std::vector<int> ranges = terrain_json["lodRanges"];
				size_t rangeCount = terrain_json["lodRangeCount"];

				terrainConfig.lodRanges = ranges;
				for (size_t i = 0; i < ranges.size(); i++)
				{
					int range = ranges[i];

					if (range == 0)
						terrainConfig.lodMorphingAreas.push_back(0);
					else
						terrainConfig.lodMorphingAreas.push_back(range - terrainConfig.UpdateMorphingArea(int(i) + 1));
				}
			}

			{
				nlohmann::json water_json = file_json["water"];
				waterConfig.scaleXZ = water_json["scaleXZ"];

				nlohmann::json tessellation_json = water_json["tessellation"];
				waterConfig.tessellationFactor = tessellation_json["factor"];
				waterConfig.tessellationSlope = tessellation_json["slope"];
				waterConfig.tessellationShift = tessellation_json["shift"];

				std::vector<int> ranges = water_json["lodRanges"];
				size_t rangeCount = water_json["lodRangeCount"];

				waterConfig.lodRanges = ranges;
				for (size_t i = 0; i < ranges.size(); i++)
				{
					int range = ranges[i];

					if (range == 0)
						waterConfig.lodMorphingAreas.push_back(0);
					else
						waterConfig.lodMorphingAreas.push_back(range - waterConfig.UpdateMorphingArea(int(i) + 1));
				}

				nlohmann::json surfaceParameters_json = water_json["surfaceParameters"];
				std::vector<float> baseColours = surfaceParameters_json["baseColour"];
				waterConfig.baseColour = { baseColours[0], baseColours[1], baseColours[2] };
				waterConfig.displacementScale = surfaceParameters_json["displacementScale"];
				waterConfig.displacementRange = surfaceParameters_json["displacementRange"];
				waterConfig.tiling = surfaceParameters_json["tiling"];
				waterConfig.choppiness = surfaceParameters_json["choppiness"];
				waterConfig.wavemotion = surfaceParameters_json["wavemotion"];
				waterConfig.normalStrength = surfaceParameters_json["normalStrength"];
				waterConfig.highDetailRange = surfaceParameters_json["highDetailRange"];
				waterConfig.capillarStrength = surfaceParameters_json["capillarStrength"];
				waterConfig.capillarDownsampling = surfaceParameters_json["capillarDownsampling"];
				
				nlohmann::json reflectionParameters_json = water_json["reflectionParameters"];
				waterConfig.fresnelFactor = reflectionParameters_json["fresnelFactor"];
				waterConfig.kReflection = reflectionParameters_json["kReflection"];
				waterConfig.kRefraction = reflectionParameters_json["kRefraction"];
				waterConfig.reflectionBlendfactor = reflectionParameters_json["reflectionBlendfactor"];
				waterConfig.dudvDownsampling = reflectionParameters_json["dudvDownsampling"];
				waterConfig.underwaterBlurfactor = reflectionParameters_json["underwaterBlurfactor"];
				waterConfig.distortionDelta = reflectionParameters_json["distortionDelta"];

				nlohmann::json lightParameters_json = water_json["lightParameters"];
				waterConfig.specularFactor = lightParameters_json["specularFactor"];
				waterConfig.specularAmplifier = lightParameters_json["specularAmplifier"];
				waterConfig.emissionFactor = lightParameters_json["emissionFactor"];
				waterConfig.diffuse = lightParameters_json["diffuseEnable"];

				nlohmann::json fftParameters_json = water_json["fftParameters"];
				std::vector<float> windDirections = fftParameters_json["windDirection"];
				waterConfig.windDirection = { windDirections[0], windDirections[1] };
				waterConfig.windSpeed = fftParameters_json["windSpeed"];
				waterConfig.alignment = fftParameters_json["alignment"];
				waterConfig.fftResolution = fftParameters_json["fftResolution"];
				waterConfig.fftAmplitude = fftParameters_json["fftAmplitude"];
				waterConfig.fftL = fftParameters_json["fftL"];
				waterConfig.fftCapillarWavesSupression = fftParameters_json["fftCapillarWavesSuppression"];
				waterConfig.choppy = fftParameters_json["choppy"];
				waterConfig.tDelta = fftParameters_json["tDelta"];
			}
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", path.c_str());
		}
	}
};