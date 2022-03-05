#include "Includes.hpp"
#include "InitDevice.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	InitDevice* InitDevice::instance;

	InitDevice& InitDevice::getInstance()
	{
		if (instance == nullptr)
		{
			if (__FrameworkConfig.api == OpenGL)
			{
				PR_LOG_WARNING("CREATE OPENGL DEVICE PROPERTIES!!!!!!!!!");
			}
			else if (__FrameworkConfig.api == Vulkan)
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
