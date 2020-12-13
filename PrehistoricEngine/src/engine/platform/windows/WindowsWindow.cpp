#include "Includes.hpp"
#include "WindowsWindow.h"

#include "platform/opengl/framework/context/GLContext.h"
#include "platform/vulkan/framework/context/VKContext.h"

#include "platform/opengl/framework/swapchain/GLSwapchain.h"
#include "platform/vulkan/framework/swapchain/VKSwapchain.h"

#include "WindowsInput.h"

#include "prehistoric/core/events/KeyEvent.h"
#include "prehistoric/core/events/MouseEvent.h"

namespace Prehistoric
{
	static bool window_size_callback(WindowResizeEvent& event)
	{
		if (event.getWidth() == 0 || event.getHeight() == 0)
		{
			return false;
		}

		Window* window = (Window*)glfwGetWindowUserPointer((GLFWwindow*)event.getHandle());
		window->getSwapchain()->SetWindowSize(event.getWidth(), event.getHeight());
		return true;
	}

	static bool window_close_callback(WindowCloseEvent& event)
	{
		Window* window = (Window*)glfwGetWindowUserPointer((GLFWwindow*)event.getHandle());
		window->setClosed(true);
		return true;
	}

	static bool window_focus_callback(WindowFocusEvent& event)
	{
		InputInstance.setPause(false);
		return true;
	}

	static bool window_focus_lost_callback(WindowLostFocusEvent& event)
	{
		InputInstance.setPause(true);
		return true;
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

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

		//Input is static so we can use it in the callbacks, but window is not, so we need a way to get it
		glfwSetWindowUserPointer(window, (void*)this);

		ImageData data = TextureLoader::LoadTextureData("res/textures/logo.png");

		GLFWimage image;
		image.width = data.width;
		image.height = data.height;
		image.pixels = data.ptr.dataUC;

		glfwSetWindowIcon(window, 1, &image);

		glfwShowWindow(window);
		SetVSync(FrameworkConfig::windowVSync); 

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);
			wnd->setWidth(width);
			wnd->setHeight(height);

			WindowResizeEvent ev(width, height, window);
			wnd->getEventCallback()((Event&)ev);
		});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			WindowCloseEvent ev(window);
			wnd->getEventCallback()((Event&)ev);
		});

		glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			if (focused == GLFW_TRUE)
			{
				WindowFocusEvent ev(window);
				wnd->getEventCallback()((Event&)ev);
			}
			else
			{
				WindowLostFocusEvent ev(window);
				wnd->getEventCallback()((Event&)ev);
			}
		});

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

	void WindowsWindow::OnEvent(Event& event)
	{
		EventDispatcher d(event);
		d.Dispatch<WindowResizeEvent>(window_size_callback);
		d.Dispatch<WindowCloseEvent>(window_close_callback);
		d.Dispatch<WindowFocusEvent>(window_focus_callback);
		d.Dispatch<WindowLostFocusEvent>(window_focus_lost_callback);

		InputInstance.OnEvent(event);
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
};