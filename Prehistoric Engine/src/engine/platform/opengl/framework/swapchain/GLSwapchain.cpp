#include "engine/prehistoric/core/util/Includes.hpp"
#include "GLSwapChain.h"
#include "engine/config/FrameworkConfig.h"
#include "engine/prehistoric/common/framework/Window.h"

GLSwapchain::GLSwapchain(Window* window)
	: Swapchain(window)
{
	//Enabling GL_TEXTURE_2D is deprecated, in debug mode, it will cause the program to break
	//glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
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
	glViewport(0, 0, width, height);
	FrameworkConfig::windowWidth = width;
	FrameworkConfig::windowHeight = height;
}