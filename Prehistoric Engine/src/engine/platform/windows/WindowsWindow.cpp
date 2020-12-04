#include "engine/prehistoric/core/util/Includes.hpp"
#include "WindowsWindow.h"

#include "engine/platform/opengl/framework/context/GLContext.h"
#include "engine/platform/vulkan/framework/context/VKContext.h"

#include "engine/platform/opengl/framework/swapchain/GLSwapchain.h"
#include "engine/platform/vulkan/framework/swapchain/VKSwapchain.h"

#include "WindowsInput.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	InputInstance.setPause(false);

	if (width == 0 || height == 0)
	{
		InputInstance.setPause(true);
		return;
	}

	Window* wnd = (Window*)glfwGetWindowUserPointer(window);
	wnd->setWidth(width);
	wnd->setHeight(height);
	wnd->getSwapchain()->SetWindowSize(width, height);
}

static void error_callback(int error, const char* description)
{
	PR_LOG_ERROR("Error: %s\n", description);
}

bool WindowsWindow::Create()
{
	//Initialisation of GLFW
	if (!initGLFW())
	{
		return false;
	}

	//Setting error callback
	glfwSetErrorCallback(error_callback);

	//Setting window properties and creating the window
	if (FrameworkConfig::api == OpenGL)
	{
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	}
	else
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, FrameworkConfig::apiVersion.x);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, FrameworkConfig::apiVersion.y);
	glfwWindowHint(GLFW_RESIZABLE, FrameworkConfig::windowResizable);
	glfwWindowHint(GLFW_SAMPLES, FrameworkConfig::windowNumSamples);

	window = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

	//Checking if a valid window was created
	if (window == NULL)
	{
		PR_LOG_ERROR("The creation of the window has failed!\n");

		glfwTerminate();
		return false;
	}
	closed = false;

	if (FrameworkConfig::api == OpenGL)
	{
		context = std::make_unique<GLContext>(this);
		swapchain = std::make_unique<GLSwapchain>(this);
	}
	else if (FrameworkConfig::api == Vulkan)
	{
		context = std::make_unique<VKContext>(this);
		swapchain = std::make_unique<VKSwapchain>(this);
	}

	ImageData data = TextureLoader::LoadTextureData("res/textures/logo.png");

	GLFWimage image;
	image.width = data.width;
	image.height = data.height;
	image.pixels = data.data;

	glfwSetWindowIcon(window, 1, &image);

	glfwShowWindow(window);
	SetVSync(FrameworkConfig::windowVSync);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return true;
}

WindowsWindow::~WindowsWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool WindowsWindow::initGLFW() const
{
	if (!glfwInit())
	{
		PR_LOG_ERROR("The initialisiation of OpenGL Framework (GLFW) has failed!\n");
		return false;
	}

	return true;
}

void WindowsWindow::Input()
{
	glfwPollEvents();
}

void WindowsWindow::Render(CommandBuffer* buffer) const
{
	swapchain->SwapBuffers(buffer);
}

void WindowsWindow::SetFullscreen(bool fullscreen)
{
	if (IsFullscreen() == fullscreen)
	{
		return;
	}

	if (fullscreen)
	{
		glfwGetWindowPos(window, &oldX, &oldY);
		glfwGetWindowSize(window, &oldWidth, &oldHeight);

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		
		width = mode->width;
		height = mode->height;

		glfwSetWindowMonitor(window, glfwGetWindowMonitor(window), 0, 0, width, height, 0);
	}
	else
	{
		width = oldWidth;
		height = oldHeight;

		glfwSetWindowMonitor(window, nullptr, oldX, oldY, width, height, 0);
	}

	this->fullscreen = fullscreen;
	swapchain->SetWindowSize(width, height);
}

bool WindowsWindow::IsFullscreen() const
{
	return fullscreen;
}

bool WindowsWindow::ShouldClose() const
{
	return glfwWindowShouldClose(window) || closed;
}