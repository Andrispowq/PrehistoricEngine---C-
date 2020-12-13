#include "Includes.hpp"
#include "WindowsInput.h"

namespace Prehistoric
{
	static bool key_pressed_callback(KeyPressedEvent& event)
	{
		auto pushedKeys = InputInstance.getPushedKeys();
		auto keysHolding = InputInstance.getKeysHolding();

		int key_code = (int)event.getKeyCode();
		if (std::find(pushedKeys.begin(), pushedKeys.end(), key_code) == pushedKeys.end())
		{
			pushedKeys.push_back(key_code);
		}
		if (std::find(keysHolding.begin(), keysHolding.end(), key_code) == keysHolding.end())
		{
			keysHolding.push_back(key_code);
		}

		InputInstance.setPushedKeys(pushedKeys);
		InputInstance.setKeysHolding(keysHolding);

		return true;
	}

	static bool key_released_callback(KeyReleasedEvent& event)
	{
		auto pushedKeys = InputInstance.getPushedKeys();
		auto keysHolding = InputInstance.getKeysHolding();

		int key_code = (int)event.getKeyCode();

		auto index0 = std::find(pushedKeys.begin(), pushedKeys.end(), key_code);
		auto index1 = std::find(keysHolding.begin(), keysHolding.end(), key_code);

		if (index0 != pushedKeys.end())
		{
			pushedKeys.erase(index0);
		}

		if (index1 != keysHolding.end())
		{
			keysHolding.erase(index1);
		}

		InputInstance.setPushedKeys(pushedKeys);
		InputInstance.setKeysHolding(keysHolding);

		return true;
	}

	static bool mouse_button_pressed_callback(MouseButtonPressedEvent& event)
	{
		auto pushedButtons = InputInstance.getPushedButtons();
		auto buttonsHolding = InputInstance.getButtonsHolding();

		int code = (int)event.getMouseButton();
		if (code == 2)
		{
			InputInstance.setLockedCursorPosition(InputInstance.getCursorPosition());
			glfwSetInputMode((GLFWwindow*)event.getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

		if (std::find(pushedButtons.begin(), pushedButtons.end(), code) == pushedButtons.end())
		{
			pushedButtons.push_back(code);
		}
		if (std::find(buttonsHolding.begin(), buttonsHolding.end(), code) == buttonsHolding.end())
		{
			buttonsHolding.push_back(code);
		}

		InputInstance.setPushedButtons(pushedButtons);
		InputInstance.setButtonsHolding(buttonsHolding);

		return true;
	}

	static bool mouse_button_released_callback(MouseButtonReleasedEvent& event)
	{
		auto pushedButtons = InputInstance.getPushedButtons();
		auto buttonsHolding = InputInstance.getButtonsHolding();

		int code = (int)event.getMouseButton();
		if (code == 2)
		{
			glfwSetInputMode((GLFWwindow*)event.getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		auto index0 = std::find(pushedButtons.begin(), pushedButtons.end(), code);
		auto index1 = std::find(buttonsHolding.begin(), buttonsHolding.end(), code);

		if (index0 != pushedButtons.end())
		{
			pushedButtons.erase(index0);
		}

		if (index1 != buttonsHolding.end())
		{
			buttonsHolding.erase(index1);
		}

		InputInstance.setPushedButtons(pushedButtons);
		InputInstance.setButtonsHolding(buttonsHolding);

		return true;
	}

	static bool mouse_moved_callback(MouseMovedEvent& event)
	{
		InputInstance.setCursorPosition(event.getPosition());
		return true;
	}

	static bool mouse_scrolled_callback(MouseScrolledEvent& event)
	{
		InputInstance.setScrollOffset(event.getOffset().y);
		return true;
	}

	bool WindowsInput::Init(Window* window) const
	{
		GLFWwindow* id = reinterpret_cast<GLFWwindow*>(window->getWindowHandle());

		glfwSetKeyCallback(id, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent ev((InputCode)key, window, 0);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent ev((InputCode)key, window, 1);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent ev((InputCode)key, window);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(id, [](GLFWwindow* window, int button, int action, int mods)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent ev((InputCode)button, window, 0);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
				case GLFW_REPEAT:
				{
					MouseButtonPressedEvent ev((InputCode)button, window, 1);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent ev((InputCode)button, window);
					wnd->getEventCallback()((Event&)ev);
					break;
				}
			}
		});

		glfwSetCursorPosCallback(id, [](GLFWwindow* window, double xPos, double yPos)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			MouseMovedEvent ev({ (float)xPos, (float)yPos }, window);
			wnd->getEventCallback()((Event&)ev);
		});

		glfwSetScrollCallback(id, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			Window* wnd = (Window*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent ev({ (float)xOffset, (float)yOffset }, window);
			wnd->getEventCallback()((Event&)ev);
		});

		return true;
	}

	bool WindowsInput::Update()
	{
		scrollOffset = 0;

		pushedKeys.clear();
		pushedButtons.clear();

		//Set up joystick/gamepad
		for (uint32_t i = 0; i < MAX_NUM_JOYSTICKS; i++)
		{
			joystickButtons[i].clear();
			joystickAxes[i].clear();

			if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i))
			{
				const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1 + i);

				if (name == NULL)
					PR_LOG_ERROR("There is no joystick/gamepad number %i present!\n", i + 1);

				if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i))
				{
					GLFWgamepadstate state;
					glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state);

					for (uint32_t j = 0; j < sizeof(state.buttons) / sizeof(state.buttons[0]); j++)
					{
						joystickButtons[i].push_back(state.buttons[j]);
					}

					for (uint32_t j = 0; j < sizeof(state.axes) / sizeof(state.axes[0]); j++)
					{
						joystickAxes[i].push_back(state.axes[j]);
					}
				}
				else
				{
					PR_LOG_WARNING("Joystick %i is not a gamepad, and thus cannot be used by the game!\n", i + 1);
				}
			}
		}

		return true;
	}

	void WindowsInput::OnEvent(Event& event)
	{
		EventDispatcher d(event);
		d.Dispatch<KeyPressedEvent>(key_pressed_callback);
		d.Dispatch<KeyReleasedEvent>(key_released_callback);
		d.Dispatch<MouseButtonPressedEvent>(mouse_button_pressed_callback);
		d.Dispatch<MouseButtonReleasedEvent>(mouse_button_released_callback);
		d.Dispatch<MouseMovedEvent>(mouse_moved_callback);
		d.Dispatch<MouseScrolledEvent>(mouse_scrolled_callback);
	}

	void WindowsInput::setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition)
	{
		glfwSetCursorPos(static_cast<GLFWwindow*>(window->getWindowHandle()), cursorPosition.x, cursorPosition.y);
	}
};