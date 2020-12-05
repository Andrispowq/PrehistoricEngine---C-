#include "engine/prehistoric/core/util/Includes.hpp"
#include "Capabilities.h"

#include "engine/platform/opengl/capabilities/GLCapabilities.h"
#include "engine/platform/vulkan/capabilities/VKCapabilities.h"

namespace Prehistoric
{
	Capabilities* Capabilities::instance = nullptr;

	Capabilities* Capabilities::getInstance()
	{
		if (instance == nullptr)
		{
			if (FrameworkConfig::api == OpenGL)
			{
				instance = new GLCapabilities();
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				instance = new VKCapabilities();
			}
		}

		return instance;
	}
};