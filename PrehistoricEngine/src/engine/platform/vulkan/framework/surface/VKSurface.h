#ifndef VK_SURFACE_H
#define VK_SURFACE_H

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "prehistoric/common/framework/Window.h"
#include "platform/vulkan/framework/context/VKInstance.h"

namespace Prehistoric
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		bool set = false;
	};

	class VKSurface
	{
	public:
		VKSurface(Window* window, VkInstance instance);
		virtual ~VKSurface();

		SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		VkSurfaceKHR getSurface() const { return surface; }

	private:
		VkSurfaceKHR surface;
		SwapchainSupportDetails swapchainSupport;

		VkInstance instance;
	};
};

#endif