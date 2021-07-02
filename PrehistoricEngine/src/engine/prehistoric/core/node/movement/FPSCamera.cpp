#include "Includes.hpp"
#include "FPSCamera.h"

#include "imgui.h"

namespace Prehistoric
{
	static const Vector3f yAxis(0, 1, 0);

	void FPSCamera::UpdateCamera(Window* window, float delta)
	{
		movAmt += InputInstance.getScrollOffset() * delta * 35.0f;
		movAmt = std::fmax(0.02f, movAmt);

		bool movedForward = false, movedBackward = false, movedRight = false, movedLeft = false,
			rotUp = false, rotDown = false, rotRight = false, rotLeft = false;

		for (CameraInput& in : inputs)
		{
			if (in.isForward() != 0 && !movedForward)
			{
				Move(forward, -movAmt * in.isForward() * delta);
				movedForward = true;
			}
			if (in.isBackward() != 0 && !movedBackward)
			{
				Move(forward, movAmt * in.isBackward() * delta);
				movedBackward = true;
			}
			if (in.isLeft() != 0 && !movedLeft)
			{
				Move(getLeft(), movAmt * in.isLeft() * delta);
				movedLeft = true;
			}
			if (in.isRight() != 0 && !movedRight)
			{
				Move(getLeft(), -movAmt * in.isRight() * delta);
				movedRight = true;
			}

			if (in.isUp() != 0 && !rotUp)
			{
				RotateX(static_cast<float>(rotAmt * 2.0 * in.isUp() * delta));
				rotUp = true;
			}
			if (in.isDown() != 0 && !rotDown)
			{
				RotateX(static_cast<float>(-rotAmt * 2.0 * in.isDown() * delta));
				rotDown = true;
			}
			if (in.isRightRot() != 0 && !rotRight)
			{
				RotateY(static_cast<float>(-rotAmt * 2.0 * in.isRightRot() * delta));
				rotRight = true;
			}
			if (in.isLeftRot() != 0 && !rotLeft)
			{
				RotateY(static_cast<float>(rotAmt * 2.0 * in.isLeftRot() * delta));
				rotLeft = true;
			}
		}

		//Free mouse rotation
		if (InputInstance.IsButtonHeld((InputCode)2))
		{
			float dy = InputInstance.getLockedCursorPosition().y - InputInstance.getCursorPosition().y;
			float dx = InputInstance.getLockedCursorPosition().x - InputInstance.getCursorPosition().x;

			//y-axis rotation
			if (dy != 0)
			{
				rotYstride = std::fabs(dy * 0.01f);
				rotYamt = -dy;
				rotYcounter = 0;
				rotYInitiated = true;
			}

			if (rotYInitiated)
			{
				//up-rotation
				if (rotYamt < 0)
				{
					if (rotYcounter > rotYamt)
					{
						RotateX(rotYstride * mouseSensitivity); //
						rotYcounter -= rotYstride;
						rotYstride *= 0.98f;
					}
					else
					{
						rotYInitiated = false;
					}
				}
				//down-rotation
				else if (rotYamt > 0)
				{
					if (rotYcounter < rotYamt)
					{
						RotateX(-rotYstride * mouseSensitivity); //
						rotYcounter += rotYstride;
						rotYstride *= 0.98f;
					}
					else
					{
						rotYInitiated = false;
					}
				}
			}

			//x-axis rotation
			if (dx != 0)
			{
				rotXstride = std::fabs(dx * 0.01f);
				rotXamt = -dx;
				rotXcounter = 0;
				rotXInitiated = true;
			}

			if (rotXInitiated)
			{
				//up-rotation
				if (rotXamt < 0)
				{
					if (rotXcounter > rotXamt)
					{
						RotateY(rotXstride * mouseSensitivity); //
						rotXcounter -= rotXstride;
						rotXstride *= 0.98f;
					}
					else
					{
						rotXInitiated = false;
					}
				}
				//down-rotation
				else if (rotXamt > 0)
				{
					if (rotXcounter < rotXamt)
					{
						RotateY(-rotXstride * mouseSensitivity); //
						rotXcounter += rotXstride;
						rotXstride *= 0.98f;
					}
					else
					{
						rotXInitiated = false;
					}
				}
			}

			InputInstance.setCursorPositionOnScreen(window, InputInstance.getLockedCursorPosition());

			if (FrameworkConfig::api == OpenGL)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			}
		}

		this->previousViewMatrix = viewMatrix;
		viewMatrix = Matrix4f::View(forward, up) * Matrix4f::Translation(position * -1);
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void FPSCamera::Move(const Vector3f& dir, const float& amount)
	{
		this->position += dir * amount;
	}

	void FPSCamera::RotateX(const float& angle)
	{
		Vector3f hAxis = yAxis.cross(forward).normalise();
		this->forward = forward.rotate(hAxis, angle).normalise();
		this->up = forward.cross(hAxis).normalise();
	}

	void FPSCamera::RotateY(const float& angle)
	{
		Vector3f hAxis = yAxis.cross(forward).normalise();
		this->forward = forward.rotate(yAxis, angle).normalise();
		this->up = forward.cross(hAxis).normalise();
	}
};