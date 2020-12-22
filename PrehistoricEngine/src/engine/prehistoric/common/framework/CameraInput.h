#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "Input.h"

namespace Prehistoric
{
	enum PR_API InputType
	{
		NO_TYPE,
		KEY_PRESSED, KEY_HELD,
		MOUSE_PRESSED, MOUSE_HELD, MOUSE_SCROLL,
		JOYSTICK_BUTTON_PRESSED, JOYSTICK_AXIS_MOVED_NEGATIVE, JOYSTICK_AXIS_MOVED_POSITIVE
	};

	struct PR_API InputData
	{
		InputType type;
		InputCode code;
		JoystickID joystickNumber;
	};

	struct PR_API MoveData
	{
		bool moved;
		float amount;
	};

	static InputData DefInData = { NO_TYPE, PR_KEY_UNKNOWN, PR_JOYSTICK_1 };

	template<typename T>
	static T clamp(T val, T min, T max)
	{
		if (val > max)
			return val = max;
		else if (val < min)
			return val = min;
		else
			return val;
	}

	class PR_API CameraInput
	{
	public:
		CameraInput(InputData forward, InputData backward, InputData right, InputData left,
			InputData up, InputData down, InputData rightRot, InputData leftRot)
			: forward(forward), backward(backward), right(right), left(left),
			up(up), down(down), rightRot(rightRot), leftRot(leftRot) {}

		CameraInput() : forward(DefInData), backward(DefInData), right(DefInData), left(DefInData),
			up(DefInData), down(DefInData), rightRot(DefInData), leftRot(DefInData) {}

		virtual ~CameraInput() {}

		//Getters
		float isForward() const { return GetKey(forward); }
		float isBackward() const { return GetKey(backward); }
		float isRight() const { return GetKey(right); }
		float isLeft() const { return GetKey(left); }

		float isUp() const { return GetKey(up); }
		float isDown() const { return GetKey(down); }
		float isRightRot() const { return GetKey(rightRot); }
		float isLeftRot() const { return GetKey(leftRot); }

		//Setters
		void setForward(const InputData& forward) { this->forward = forward; }
		void setBackward(const InputData& backward) { this->backward = backward; }
		void setRight(const InputData& right) { this->right = right; }
		void setLeft(const InputData& left) { this->left = left; }

		void setUp(const InputData& up) { this->up = up; }
		void setDowny(const InputData& down) { this->down = down; }
		void setRightRot(const InputData& rightRot) { this->rightRot = rightRot; }
		void setLeftRot(const InputData& leftRot) { this->leftRot = leftRot; }

		static float GetKey(const InputData& data)
		{
			switch (data.type)
			{
			case KEY_PRESSED:
				return (float)InputInstance.IsKeyPushed(data.code);
			case KEY_HELD:
				return (float)InputInstance.IsKeyHeld(data.code);

			case MOUSE_PRESSED:
				return (float)InputInstance.IsButtonPushed(data.code);
			case MOUSE_HELD:
				return (float)InputInstance.IsButtonHeld(data.code);
			case MOUSE_SCROLL:
				return InputInstance.getScrollOffset();

			case JOYSTICK_BUTTON_PRESSED:
				return (float)InputInstance.IsJoystickButtonPushed(data.code, data.joystickNumber);
			case JOYSTICK_AXIS_MOVED_NEGATIVE:
				return clamp(-InputInstance.getJoystickAxisOffset(data.code, data.joystickNumber), -1.0f, 0.0f);
			case JOYSTICK_AXIS_MOVED_POSITIVE:
				return clamp(-InputInstance.getJoystickAxisOffset(data.code, data.joystickNumber), 0.0f, 1.0f);

			default:
				return 0;
			}
		}
	private:
		InputData forward;
		InputData backward;
		InputData right;
		InputData left;

		InputData up;
		InputData down;
		InputData rightRot;
		InputData leftRot;
	};
};

#endif