#include "Includes.hpp"
#include "WindowsInput.h"

namespace Prehistoric
{
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto pushedKeys = InputInstance.getPushedKeys();
		auto keysHolding = InputInstance.getKeysHolding();
		auto releasedKeys = InputInstance.getReleasedKeys();

		if (action == GLFW_PRESS)
		{
			if (std::find(pushedKeys.begin(), pushedKeys.end(), key) == pushedKeys.end())
			{
				pushedKeys.push_back(key);
				keysHolding.push_back(key);
			}
		}

		if (action == GLFW_RELEASE)
		{
			releasedKeys.push_back(key);

			auto index0 = std::find(pushedKeys.begin(), pushedKeys.end(), key);
			auto index1 = std::find(keysHolding.begin(), keysHolding.end(), key);

			if (index0 != pushedKeys.end())
			{
				pushedKeys.erase(index0);
			}

			if (index1 != keysHolding.end())
			{
				keysHolding.erase(index1);
			}
		}

		InputInstance.setPushedKeys(pushedKeys);
		InputInstance.setKeysHolding(keysHolding);
		InputInstance.setReleasedKeys(releasedKeys);
	}

	static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (button == 2 && action == GLFW_PRESS)
		{
			InputInstance.setLockedCursorPosition(InputInstance.getCursorPosition());
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

		if (button == 2 && action == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		auto pushedButtons = InputInstance.getPushedButtons();
		auto buttonsHolding = InputInstance.getButtonsHolding();
		auto releasedButtons = InputInstance.getReleasedButtons();

		if (action == GLFW_PRESS)
		{
			if (std::find(pushedButtons.begin(), pushedButtons.end(), button) == pushedButtons.end())
			{
				pushedButtons.push_back(button);
				buttonsHolding.push_back(button);
			}
		}

		if (action == GLFW_RELEASE)
		{
			releasedButtons.push_back(button);

			auto index0 = std::find(pushedButtons.begin(), pushedButtons.end(), button);
			auto index1 = std::find(buttonsHolding.begin(), buttonsHolding.end(), button);

			if (index0 != pushedButtons.end())
			{
				pushedButtons.erase(index0);
			}

			if (index1 != buttonsHolding.end())
			{
				buttonsHolding.erase(index1);
			}
		}

		InputInstance.setPushedButtons(pushedButtons);
		InputInstance.setButtonsHolding(buttonsHolding);
		InputInstance.setReleasedButtons(releasedButtons);
	}

	static void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
	{
		InputInstance.setCursorPosition(Vector2f(static_cast<float>(xPos), static_cast<float>(yPos)));
	}

	static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
	{
		InputInstance.setScrollOffset(static_cast<float>(yOffset));
	}

	static void focus_callback(GLFWwindow* window, int iconified)
	{
		if (iconified == GLFW_TRUE)
		{
			InputInstance.setPause(true);
		}
		else
		{
			InputInstance.setPause(false);
		}
	}

	bool WindowsInput::Init(Window* window) const
	{
		GLFWwindow* id = reinterpret_cast<GLFWwindow*>(window->getWindowHandle());

		//Input is static so we can use it in the callbacks, but window is not, so we need a way to get it
		glfwSetWindowUserPointer(id, (void*)window);

		glfwSetKeyCallback(id, key_callback);
		glfwSetMouseButtonCallback(id, mouse_callback);
		glfwSetCursorPosCallback(id, cursor_pos_callback);
		glfwSetScrollCallback(id, scroll_callback);

		glfwSetWindowFocusCallback(id, focus_callback);

		return true;
	}

	bool WindowsInput::Update()
	{
		scrollOffset = 0;

		pushedKeys.clear();
		releasedKeys.clear();

		pushedButtons.clear();
		releasedButtons.clear();

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

	void WindowsInput::setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition)
	{
		glfwSetCursorPos(static_cast<GLFWwindow*>(window->getWindowHandle()), cursorPosition.x, cursorPosition.y);
	}
};