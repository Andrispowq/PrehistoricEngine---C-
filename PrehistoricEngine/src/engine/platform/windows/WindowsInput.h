#ifndef WINDOWS_INPUT_H
#define WINDOWS_INPUT_H

#include "prehistoric/common/framework/Input.h"

#include <GLFW/glfw3.h>

#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/core/events/KeyEvent.h"
#include "prehistoric/core/events/MouseEvent.h"
#include "prehistoric/core/events/JoystickEvent.h"

namespace Prehistoric
{
	class WindowsInput : public Input
	{
	public:
		virtual bool Init(Window* window) const override;
		virtual bool Update() override;

		virtual void OnEvent(Event& event) override;

		virtual void setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition) override;
	};
};

#endif
