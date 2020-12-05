#include "Includes.hpp"
#include "InitDevice.h"

namespace Prehistoric
{
	InitDevice* InitDevice::instance;

	InitDevice& InitDevice::getInstance()
	{
		if (instance == nullptr)
		{
			if (FrameworkConfig::api == OpenGL)
			{
				PR_LOG_WARNING("CREATE OPENGL DEVICE PROPERTIES!!!!!!!!!");
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				PR_LOG_WARNING("CREATE VULKAN DEVICE PROPERTIES!!!!!!!!!");
			}
		}

		return *instance;
	}

	void InitDevice::DeleteInstance()
	{
		delete instance;
	}
};
