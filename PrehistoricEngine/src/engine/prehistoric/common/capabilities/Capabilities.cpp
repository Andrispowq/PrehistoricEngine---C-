#include "Includes.hpp"
#include "Capabilities.h"

#include "prehistoric/application/Application.h"

#include "platform/opengl/capabilities/GLCapabilities.h"
#include "platform/vulkan/capabilities/VKCapabilities.h"

namespace Prehistoric
{
	Capabilities* Capabilities::instance = nullptr;

	Capabilities* Capabilities::getInstance()
	{
		if (instance == nullptr)
		{
			if (__FrameworkConfig.api == OpenGL)
			{
				instance = new GLCapabilities();
			}
			else if (__FrameworkConfig.api == Vulkan)
			{
				instance = new VKCapabilities();
			}
		}

		return instance;
	}
};