#include "Includes.hpp"
#include "VKSurface.h"

namespace Prehistoric
{
	VKSurface::VKSurface(Window* window, VkInstance instance)
	{
		this->instance = instance;

		VkResult res;
		if ((res = glfwCreateWindowSurface(instance, (GLFWwindow*)window->getWindowHandle(), nullptr, &surface)) != VK_SUCCESS)
		{
			PR_LOG_RUNTIME_ERROR("The creation of VkSurfaceKHR has failed! Error: %i\n", res);
		}
	}

	VKSurface::~VKSurface()
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	SwapchainSupportDetails VKSurface::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainSupport.capabilities);
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			swapchainSupport.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainSupport.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			swapchainSupport.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainSupport.presentModes.data());
		}

		return swapchainSupport;
	}
};