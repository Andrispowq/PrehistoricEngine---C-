#ifndef FRAMEWORKCONFIG_H
#define FRAMEWORKCONFIG_H

#include <fstream>
#include <map>

#include "engine/prehistoric/core/util/Util.h"
#include "engine/prehistoric/core/util/math/Math.h"
#include "engine/prehistoric/core/util/log/Log.h"

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

	namespace FrameworkConfig
	{
		void LoadConfig(const std::string& path);

		extern std::string windowName;
		extern uint32_t windowWidth;
		extern uint32_t windowHeight;
		extern bool windowFullScreen;
		extern bool windowResizable;
		extern bool windowVSync;
		extern uint32_t windowNumSamples;
		extern uint32_t windowMaxFPS;

		extern std::map<API, Vector2i> apiVersions;

		extern API api;
		extern Vector2i apiVersion;
		extern bool apiVulkanUseValidationLayers;
	};
};

#endif