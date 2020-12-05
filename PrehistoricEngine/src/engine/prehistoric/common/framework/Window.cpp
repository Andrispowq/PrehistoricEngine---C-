#include "Includes.hpp"
#include "Window.h"
#include "prehistoric/common/framework/context/Context.h"
#include "prehistoric/core/config/FrameworkConfig.h"
#include "platform/opengl/framework/swapchain/GLSwapChain.h"
#include "platform/vulkan/framework/swapchain/VkSwapChain.h"
#include "platform/opengl/framework/context/GLContext.h"
#include "platform/vulkan/framework/context/VKContext.h"

namespace Prehistoric
{
	Window::Window()
		: swapchain(nullptr), context(nullptr)
	{
		this->width = FrameworkConfig::windowWidth;
		this->height = FrameworkConfig::windowHeight;
		this->title = FrameworkConfig::windowName.c_str();
		this->fullscreen = FrameworkConfig::windowFullScreen;
		this->closed = true;
	}

	Window::~Window()
	{
		delete swapchain.release();
		delete context.release();
	}
};
