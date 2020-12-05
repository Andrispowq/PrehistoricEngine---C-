#ifndef VK_SURFACE_H
#define VK_SURFACE_H

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "prehistoric/common/framework/Window.h"
#include "platform/vulkan/framework/context/VKInstance.h"

namespace Prehistoric
{
	class VKSurface
	{
	public:
		VKSurface(Window* window, VkInstance instance);
		virtual ~VKSurface();

		VkSurfaceKHR getSurface() const { return surface; }
	private:
		VkInstance instance;

		VkSurfaceKHR surface;
	};
};

#endif