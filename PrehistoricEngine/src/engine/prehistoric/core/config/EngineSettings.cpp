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

				nlohmann::json window_json = file_json["window"];
				frameworkConfig.windowName = window_json["name"];
				frameworkConfig.windowWidth = window_json["width"];
				frameworkConfig.windowHeight = window_json["height"];
				frameworkConfig.windowFullScreen = window_json["fullScreen"];
				frameworkConfig.windowResizable = window_json["resizable"];
				frameworkConfig.windowVSync = window_json["vSync"];
				frameworkConfig.windowNumSamples = window_json["numSamples"];
				frameworkConfig.windowMaxFPS = window_json["maxFPS"];

				nlohmann::json api_json = file_json["api"];
				std::string activeAPI = api_json["active"];
				if (activeAPI == "OpenGL") frameworkConfig.api = API::OpenGL;
				else if (activeAPI == "Vulkan") frameworkConfig.api = API::Vulkan;

				nlohmann::json opengl_json = api_json["OpenGL"];
				frameworkConfig.apiVersions.insert(std::make_pair(API::OpenGL, opengl_json["version"]));

				nlohmann::json vulkan_json = api_json["Vulkan"];
				frameworkConfig.apiVersions.insert(std::make_pair(API::Vulkan, vulkan_json["version"]));
				frameworkConfig.apiVulkanUseValidationLayers = vulkan_json["useValidationLayers"];
			}

			{
				nlohmann::json engine_json = file_json["engine"];
				engineConfig.lightsMaxNumber = engine_json["maxNumberOfLights"];

				nlohmann::json renderer_json = file_json["renderer"];
				engineConfig.rendererGamma = renderer_json["gamma"];
				engineConfig.rendererExposure = renderer_json["exposure"];
				engineConfig.rendererHighDetailRange = renderer_json["highDetailRange"];
				engineConfig.rendererNearPlane = renderer_json["nearPlane"];
				engineConfig.rendererFarPlane = renderer_json["farPlane"];
			}

			{
				nlohmann::json environment_map_json = file_json["environmentMap"];
				environmentMapConfig.environmentMapLocation = file_json["location"];
				environmentMapConfig.environmentMapResolution = file_json["environmentMapResolution"];
				environmentMapConfig.irradianceMapResolution = file_json["irradianceMapResolution"];
				environmentMapConfig.prefilterMapResolution = file_json["prefilterMapResolution"];
				environmentMapConfig.prefilterLevels = file_json["prefilterMapLevels"];
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
						terrainConfig.lodMorphingAreas.push_back(range - terrainConfig.UpdateMorphingArea(i + 1));

				}
			}
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", path.c_str());
		}
	}
};