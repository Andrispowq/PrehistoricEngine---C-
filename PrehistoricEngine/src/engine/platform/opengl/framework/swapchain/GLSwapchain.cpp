#include "Includes.hpp"
#include "GLSwapChain.h"
#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	GLSwapchain::GLSwapchain(Window* window)
		: Swapchain(window)
	{
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_MULTISAMPLE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	GLSwapchain::~GLSwapchain()
	{
	}

	void GLSwapchain::SwapBuffers(CommandBuffer* buffer)
	{
		glfwSwapBuffers((GLFWwindow*)window->getWindowHandle());
	}

	void GLSwapchain::ClearScreen()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0);
	}

	void GLSwapchain::SetVSync(bool vSync) const
	{
		glfwSwapInterval(vSync ? 1 : 0);
	}

	void GLSwapchain::SetWindowSize(uint32_t width, uint32_t height)
	{
		PR_LOG(GREEN, "%d by %d\n", width, height);
		glViewport(0, 0, width, height);
		FrameworkConfig::windowWidth = width;
		FrameworkConfig::windowHeight = height;
	}
};