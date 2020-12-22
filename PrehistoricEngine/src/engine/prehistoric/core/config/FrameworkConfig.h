#ifndef FRAMEWORKCONFIG_H
#define FRAMEWORKCONFIG_H

#include <fstream>
#include <map>

#include "prehistoric/core/util/Util.h"
#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/core/util/log/Log.h"

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
	enum PR_API API
	{
		OpenGL, Vulkan, Vulkan_RTX, DirectX_11, DirectX_12, DirectX_DXR, NO_API
	};

	namespace FrameworkConfig
	{
		PR_API void LoadConfig(const std::string& path);

		extern PR_API std::string windowName;
		extern PR_API uint32_t windowWidth;
		extern PR_API uint32_t windowHeight;
		extern PR_API bool windowFullScreen;
		extern PR_API bool windowResizable;
		extern PR_API bool windowVSync;
		extern PR_API uint32_t windowNumSamples;
		extern PR_API uint32_t windowMaxFPS;

		extern PR_API std::map<API, Vector2i> apiVersions;

		extern PR_API API api;
		extern PR_API Vector2i apiVersion;
		extern PR_API bool apiVulkanUseValidationLayers;
	};
};

#endif