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

		if (buttons.size() == 0)
		{
			return 0;
		}

		return buttons[(uint32_t)key] == 1;
	}

	float Input::getJoystickAxisOffset(const InputCode& axis, const JoystickID& joystick) const
	{
		auto axes = joystickAxes[(uint32_t)joystick];

		if (axes.size() == 0)
		{
			return 0;
		}

		float ret = axes[(uint32_t)axis];
		if (std::abs(ret) < 0.2f)
		{
			return 0.0f;
		}

		return ret;
	}
};