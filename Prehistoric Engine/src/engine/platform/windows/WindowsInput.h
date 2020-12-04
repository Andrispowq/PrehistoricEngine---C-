#ifndef WINDOWS_INPUT_H
#define WINDOWS_INPUT_H

#include "engine/prehistoric/common/framework/Input.h"

#include <glfw3.h>

#include "engine/prehistoric/core/math/Math.h"

class WindowsInput : public Input
{
public:
	bool Init(Window* window) const override;
	bool Update() override;

	void setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition) override;
};

#endif
