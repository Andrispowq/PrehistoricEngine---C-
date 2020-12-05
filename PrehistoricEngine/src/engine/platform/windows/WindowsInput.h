#ifndef WINDOWS_INPUT_H
#define WINDOWS_INPUT_H

#include "prehistoric/common/framework/Input.h"

#include <glfw3.h>

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class WindowsInput : public Input
	{
	public:
		bool Init(Window* window) const override;
		bool Update() override;

		void setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition) override;
	};
};

#endif
