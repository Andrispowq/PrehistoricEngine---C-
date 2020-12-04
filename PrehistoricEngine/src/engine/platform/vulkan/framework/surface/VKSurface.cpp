#include "engine/prehistoric/core/util/Includes.hpp"
#include "VKSurface.h"

VKSurface::VKSurface(Window* window, VkInstance instance)
{
	this->instance = instance;

	VkResult res;
	if ((res = glfwCreateWindowSurface(instance, (GLFWwindow*) window->getWindowHandle(), nullptr, &surface)) != VK_SUCCESS)
	{
		PR_LOG_RUNTIME_ERROR("The creation of VkSurfaceKHR has failed! Error: %i\n", res);
	}
}

VKSurface::~VKSurface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}