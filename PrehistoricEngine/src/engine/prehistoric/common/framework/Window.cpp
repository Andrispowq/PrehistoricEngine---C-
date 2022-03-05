#include "Includes.hpp"
#include "Window.h"

#include "prehistoric/application/Application.h"

#include "prehistoric/common/framework/context/Context.h"
#include "platform/opengl/framework/swapchain/GLSwapChain.h"
#include "platform/vulkan/framework/swapchain/VkSwapChain.h"
#include "platform/opengl/framework/context/GLContext.h"
#include "platform/vulkan/framework/context/VKContext.h"

namespace Prehistoric
{
	Window::Window()
		: swapchain(nullptr), context(nullptr), resized(false)
	{
		this->width = __FrameworkConfig.windowWidth;
		this->height = __FrameworkConfig.windowHeight;
		this->title = __FrameworkConfig.windowName.c_str();
		this->fullscreen = __FrameworkConfig.windowFullScreen;
		this->closed = true;
	}

	Window::~Window()
	{
		delete swapchain.release();
		delete context.release();
	}
};
