#include "engine/prehistoric/core/util/Includes.hpp"
#include "Window.h"
#include "engine/prehistoric/common/framework/context/Context.h"
#include "engine/prehistoric/core/config/FrameworkConfig.h"
#include "engine/platform/opengl/framework/swapchain/GLSwapChain.h"
#include "engine/platform/vulkan/framework/swapchain/VkSwapChain.h"
#include "engine/platform/opengl/framework/context/GLContext.h"
#include "engine/platform/vulkan/framework/context/VKContext.h"

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
