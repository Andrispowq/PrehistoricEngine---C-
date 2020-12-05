#include "Includes.hpp"
#include "Input.h"
#include "platform/Platform.h"
#include "platform/windows/WindowsInput.h"

namespace Prehistoric
{
	Input* Input::instance = nullptr;

	Input& Input::getInstance()
	{
		if (instance == nullptr)
		{
#if defined(PR_WINDOWS_64)
			instance = new WindowsInput();
#endif
		}

		return *instance;
	}

	void Input::DeleteInstance()
	{
		delete instance;
	}

	bool Input::IsJoystickButtonPushed(const InputCode& key, const JoystickID& joystick) const
	{
		auto buttons = joystickButtons[(uint32_t)joystick];

		if (std::find(buttons.begin(), buttons.end(), (int32_t)key) != buttons.end())
		{
			return buttons[(uint32_t)key] == 1;
		}

		return false;
	}

	float Input::getJoystickAxisOffset(const InputCode& axis, const JoystickID& joystick) const
	{
		auto axes = joystickAxes[(uint32_t)joystick];

		if (std::find(axes.begin(), axes.end(), (float)axis) != axes.end())
		{
			return axes[(uint32_t)axis];
		}

		return 0;
	}
};