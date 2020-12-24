#ifndef INPUT_CODE_H
#define INPUT_CODE_H

//Everything in this class is from GLFW/glfw3.h

#include "Includes.hpp"

namespace Prehistoric
{
	enum class InputCode : uint16_t
	{
		/* The unknown key */
		KEY_UNKNOWN = 0xFFFF,

		/* Printable keys */
		KEY_SPACE = 32,
		KEY_APOSTROPHE = 39,  /* ' */
		KEY_COMMA = 44,  /* , */
		KEY_MINUS = 45,  /* - */
		KEY_PERIOD = 46,  /* . */
		KEY_SLASH = 47,  /* / */
		KEY_0 = 48,
		KEY_1 = 49,
		KEY_2 = 50,
		KEY_3 = 51,
		KEY_4 = 52,
		KEY_5 = 53,
		KEY_6 = 54,
		KEY_7 = 55,
		KEY_8 = 56,
		KEY_9 = 57,
		KEY_SEMICOLON = 59,  /* ; */
		KEY_EQUAL = 61,  /* =  */
		KEY_A = 65,
		KEY_B = 66,
		KEY_C = 67,
		KEY_D = 68,
		KEY_E = 69,
		KEY_F = 70,
		KEY_G = 71,
		KEY_H = 72,
		KEY_I = 73,
		KEY_J = 74,
		KEY_K = 75,
		KEY_L = 76,
		KEY_M = 77,
		KEY_N = 78,
		KEY_O = 79,
		KEY_P = 80,
		KEY_Q = 81,
		KEY_R = 82,
		KEY_S = 83,
		KEY_T = 84,
		KEY_U = 85,
		KEY_V = 86,
		KEY_W = 87,
		KEY_X = 88,
		KEY_Y = 89,
		KEY_Z = 90,
		KEY_LEFT_BRACKET = 91,  /* [ */
		KEY_BACKSLASH = 92,  /* \ */
		KEY_RIGHT_BRACKET = 93,  /* ] */
		KEY_GRAVE_ACCENT = 96,  /* ` */
		KEY_WORLD_1 = 161, /* non-US #1 */
		KEY_WORLD_2 = 162, /* non-US #2 */

		/* Function keys */
		KEY_ESCAPE = 256,
		KEY_ENTER = 257,
		KEY_TAB = 258,
		KEY_BACKSPACE = 259,
		KEY_INSERT = 260,
		KEY_DELETE = 261,
		KEY_RIGHT = 262,
		KEY_LEFT = 263,
		KEY_DOWN = 264,
		KEY_UP = 265,
		KEY_PAGE_UP = 266,
		KEY_PAGE_DOWN = 267,
		KEY_HOME = 268,
		KEY_END = 269,
		KEY_CAPS_LOCK = 280,
		KEY_SCROLL_LOCK = 281,
		KEY_NUM_LOCK = 282,
		KEY_PRINT_SCREEN = 283,
		KEY_PAUSE = 284,
		KEY_F1 = 290,
		KEY_F2 = 291,
		KEY_F3 = 292,
		KEY_F4 = 293,
		KEY_F5 = 294,
		KEY_F6 = 295,
		KEY_F7 = 296,
		KEY_F8 = 297,
		KEY_F9 = 298,
		KEY_F10 = 299,
		KEY_F11 = 300,
		KEY_F12 = 301,
		KEY_F13 = 302,
		KEY_F14 = 303,
		KEY_F15 = 304,
		KEY_F16 = 305,
		KEY_F17 = 306,
		KEY_F18 = 307,
		KEY_F19 = 308,
		KEY_F20 = 309,
		KEY_F21 = 310,
		KEY_F22 = 311,
		KEY_F23 = 312,
		KEY_F24 = 313,
		KEY_F25 = 314,
		KEY_KP_0 = 320,
		KEY_KP_1 = 321,
		KEY_KP_2 = 322,
		KEY_KP_3 = 323,
		KEY_KP_4 = 324,
		KEY_KP_5 = 325,
		KEY_KP_6 = 326,
		KEY_KP_7 = 327,
		KEY_KP_8 = 328,
		KEY_KP_9 = 329,
		KEY_KP_DECIMAL = 330,
		KEY_KP_DIVIDE = 331,
		KEY_KP_MULTIPLY = 332,
		KEY_KP_SUBTRACT = 333,
		KEY_KP_ADD = 334,
		KEY_KP_ENTER = 335,
		KEY_KP_EQUAL = 336,
		KEY_LEFT_SHIFT = 340,
		KEY_LEFT_CONTROL = 341,
		KEY_LEFT_ALT = 342,
		KEY_LEFT_SUPER = 343,
		KEY_RIGHT_SHIFT = 344,
		KEY_RIGHT_CONTROL = 345,
		KEY_RIGHT_ALT = 346,
		KEY_RIGHT_SUPER = 347,
		KEY_RIGHT_WINDOWS = KEY_RIGHT_SUPER,
		KEY_RIGHT_COMMAND = KEY_RIGHT_SUPER,
		KEY_MENU = 348,

		KEY_LAST = KEY_MENU,

		MOUSE_BUTTON_1 = 0,
		MOUSE_BUTTON_2 = 1,
		MOUSE_BUTTON_3 = 2,
		MOUSE_BUTTON_4 = 3,
		MOUSE_BUTTON_5 = 4,
		MOUSE_BUTTON_6 = 5,
		MOUSE_BUTTON_7 = 6,
		MOUSE_BUTTON_8 = 7,
		MOUSE_BUTTON_LAST = MOUSE_BUTTON_8,
		MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
		MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
		MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3,

		GAMEPAD_BUTTON_A = 0,
		GAMEPAD_BUTTON_B = 1,
		GAMEPAD_BUTTON_X = 2,
		GAMEPAD_BUTTON_Y = 3,
		GAMEPAD_BUTTON_LEFT_BUMPER = 4,
		GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
		GAMEPAD_BUTTON_BACK = 6,
		GAMEPAD_BUTTON_START = 7,
		GAMEPAD_BUTTON_GUIDE = 8,
		GAMEPAD_BUTTON_LEFT_THUMB = 9,
		GAMEPAD_BUTTON_RIGHT_THUMB = 10,
		GAMEPAD_BUTTON_DPAD_UP = 11,
		GAMEPAD_BUTTON_DPAD_RIGHT = 12,
		GAMEPAD_BUTTON_DPAD_DOWN = 13,
		GAMEPAD_BUTTON_DPAD_LEFT = 14,
		GAMEPAD_BUTTON_LAST = GAMEPAD_BUTTON_DPAD_LEFT,

		GAMEPAD_BUTTON_CROSS = GAMEPAD_BUTTON_A,
		GAMEPAD_BUTTON_CIRCLE = GAMEPAD_BUTTON_B,
		GAMEPAD_BUTTON_SQUARE = GAMEPAD_BUTTON_X,
		GAMEPAD_BUTTON_TRIANGLE = GAMEPAD_BUTTON_Y,

		GAMEPAD_AXIS_LEFT_X = 0,
		GAMEPAD_AXIS_LEFT_Y = 1,
		GAMEPAD_AXIS_RIGHT_X = 2,
		GAMEPAD_AXIS_RIGHT_Y = 3,
		GAMEPAD_AXIS_LEFT_TRIGGER = 4,
		GAMEPAD_AXIS_RIGHT_TRIGGER = 5,
		GAMEPAD_AXIS_LAST = GAMEPAD_AXIS_RIGHT_TRIGGER
	};

	enum class JoystickID : uint16_t
	{
		JOYSTICK_1 = 0,
		JOYSTICK_2 = 1,
		JOYSTICK_3 = 2,
		JOYSTICK_4 = 3,
		JOYSTICK_5 = 4,
		JOYSTICK_6 = 5,
		JOYSTICK_7 = 6,
		JOYSTICK_8 = 7,
		JOYSTICK_9 = 8,
		JOYSTICK_10 = 9,
		JOYSTICK_11 = 10,
		JOYSTICK_12 = 11,
		JOYSTICK_13 = 12,
		JOYSTICK_14 = 13,
		JOYSTICK_15 = 14,
		JOYSTICK_16 = 15,
		JOYSTICK_LAST = JOYSTICK_16
	};

};

#define PR_KEY_UNKNOWN					Prehistoric::InputCode::KEY_UNKNOWN

#define PR_KEY_SPACE					Prehistoric::InputCode::KEY_SPACE
#define PR_KEY_APOSTROPHE				Prehistoric::InputCode::KEY_APOSTROPHE
#define PR_KEY_COMMA					Prehistoric::InputCode::KEY_COMMA
#define PR_KEY_MINUS					Prehistoric::InputCode::KEY_MINUS
#define PR_KEY_PERIOD					Prehistoric::InputCode::KEY_PERIOD
#define PR_KEY_SLASH					Prehistoric::InputCode::KEY_SLASH
#define PR_KEY_0						Prehistoric::InputCode::KEY_0
#define PR_KEY_1						Prehistoric::InputCode::KEY_1
#define PR_KEY_2						Prehistoric::InputCode::KEY_2
#define PR_KEY_3						Prehistoric::InputCode::KEY_3
#define PR_KEY_4						Prehistoric::InputCode::KEY_4
#define PR_KEY_5						Prehistoric::InputCode::KEY_5
#define PR_KEY_6						Prehistoric::InputCode::KEY_6
#define PR_KEY_7						Prehistoric::InputCode::KEY_7
#define PR_KEY_8						Prehistoric::InputCode::KEY_8
#define PR_KEY_9						Prehistoric::InputCode::KEY_9
#define PR_KEY_SEMICOLON				Prehistoric::InputCode::KEY_SEMICOLON
#define PR_KEY_EQUAL					Prehistoric::InputCode::KEY_EQUAL
#define PR_KEY_A						Prehistoric::InputCode::KEY_A
#define PR_KEY_B						Prehistoric::InputCode::KEY_B
#define PR_KEY_C						Prehistoric::InputCode::KEY_C
#define PR_KEY_D						Prehistoric::InputCode::KEY_D
#define PR_KEY_E						Prehistoric::InputCode::KEY_E
#define PR_KEY_F						Prehistoric::InputCode::KEY_F
#define PR_KEY_G						Prehistoric::InputCode::KEY_G
#define PR_KEY_H						Prehistoric::InputCode::KEY_H
#define PR_KEY_I						Prehistoric::InputCode::KEY_I
#define PR_KEY_J						Prehistoric::InputCode::KEY_J
#define PR_KEY_K						Prehistoric::InputCode::KEY_K
#define PR_KEY_L						Prehistoric::InputCode::KEY_L
#define PR_KEY_M						Prehistoric::InputCode::KEY_M
#define PR_KEY_N						Prehistoric::InputCode::KEY_N
#define PR_KEY_O						Prehistoric::InputCode::KEY_O
#define PR_KEY_P						Prehistoric::InputCode::KEY_P
#define PR_KEY_Q						Prehistoric::InputCode::KEY_Q
#define PR_KEY_R						Prehistoric::InputCode::KEY_R
#define PR_KEY_S						Prehistoric::InputCode::KEY_S
#define PR_KEY_T						Prehistoric::InputCode::KEY_T
#define PR_KEY_U						Prehistoric::InputCode::KEY_U
#define PR_KEY_V						Prehistoric::InputCode::KEY_V
#define PR_KEY_W						Prehistoric::InputCode::KEY_W
#define PR_KEY_X						Prehistoric::InputCode::KEY_X
#define PR_KEY_Y						Prehistoric::InputCode::KEY_Y
#define PR_KEY_Z						Prehistoric::InputCode::KEY_Z
#define PR_KEY_LEFT_BRACKET				Prehistoric::InputCode::KEY_LEFT_BRACKET
#define PR_KEY_BACKSLASH				Prehistoric::InputCode::KEY_BACKSLASH
#define PR_KEY_RIGHT_BRACKET			Prehistoric::InputCode::KEY_RIGHT_BRACKET
#define PR_KEY_GRAVE_ACCENT				Prehistoric::InputCode::KEY_GRAVE_ACCENT
#define PR_KEY_WORLD_1					Prehistoric::InputCode::KEY_WORLD_1
#define PR_KEY_WORLD_2					Prehistoric::InputCode::KEY_WORLD_2

#define PR_KEY_ESCAPE					Prehistoric::InputCode::KEY_ESCAPE
#define PR_KEY_ENTER					Prehistoric::InputCode::KEY_ENTER
#define PR_KEY_TAB						Prehistoric::InputCode::KEY_TAB
#define PR_KEY_BACKSPACE				Prehistoric::InputCode::KEY_BACKSPACE
#define PR_KEY_INSERT					Prehistoric::InputCode::KEY_INSERT
#define PR_KEY_DELETE					Prehistoric::InputCode::KEY_DELETE
#define PR_KEY_RIGHT					Prehistoric::InputCode::KEY_RIGHT
#define PR_KEY_LEFT						Prehistoric::InputCode::KEY_LEFT
#define PR_KEY_DOWN						Prehistoric::InputCode::KEY_DOWN
#define PR_KEY_UP						Prehistoric::InputCode::KEY_UP
#define PR_KEY_PAGE_UP					Prehistoric::InputCode::KEY_PAGE_UP
#define PR_KEY_PAGE_DOWN				Prehistoric::InputCode::KEY_PAGE_DOWN
#define PR_KEY_HOME						Prehistoric::InputCode::KEY_HOME
#define PR_KEY_END						Prehistoric::InputCode::KEY_END
#define PR_KEY_CAPS_LOCK				Prehistoric::InputCode::KEY_CAPS_LOCK
#define PR_KEY_SCROLL_LOCK				Prehistoric::InputCode::KEY_SCROLL_LOCK
#define PR_KEY_NUM_LOCK					Prehistoric::InputCode::KEY_NUM_LOCK
#define PR_KEY_PRINT_SCREEN				Prehistoric::InputCode::KEY_PRINT_SCREEN
#define PR_KEY_PAUSE					Prehistoric::InputCode::KEY_PAUSE
#define PR_KEY_F1						Prehistoric::InputCode::KEY_F1
#define PR_KEY_F2						Prehistoric::InputCode::KEY_F2
#define PR_KEY_F3						Prehistoric::InputCode::KEY_F3
#define PR_KEY_F4						Prehistoric::InputCode::KEY_F4
#define PR_KEY_F5						Prehistoric::InputCode::KEY_F5
#define PR_KEY_F6						Prehistoric::InputCode::KEY_F6
#define PR_KEY_F7						Prehistoric::InputCode::KEY_F7
#define PR_KEY_F8						Prehistoric::InputCode::KEY_F8
#define PR_KEY_F9						Prehistoric::InputCode::KEY_F9
#define PR_KEY_F10						Prehistoric::InputCode::KEY_F10
#define PR_KEY_F11						Prehistoric::InputCode::KEY_F11
#define PR_KEY_F12						Prehistoric::InputCode::KEY_F12
#define PR_KEY_F13						Prehistoric::InputCode::KEY_F13
#define PR_KEY_F14						Prehistoric::InputCode::KEY_F14
#define PR_KEY_F15						Prehistoric::InputCode::KEY_F15
#define PR_KEY_F16						Prehistoric::InputCode::KEY_F16
#define PR_KEY_F17						Prehistoric::InputCode::KEY_F17
#define PR_KEY_F18						Prehistoric::InputCode::KEY_F18
#define PR_KEY_F19						Prehistoric::InputCode::KEY_F19
#define PR_KEY_F20						Prehistoric::InputCode::KEY_F20
#define PR_KEY_F21						Prehistoric::InputCode::KEY_F21
#define PR_KEY_F22						Prehistoric::InputCode::KEY_F22
#define PR_KEY_F23						Prehistoric::InputCode::KEY_F23
#define PR_KEY_F24						Prehistoric::InputCode::KEY_F24
#define PR_KEY_F25						Prehistoric::InputCode::KEY_F25
#define PR_KEY_KP_0						Prehistoric::InputCode::KEY_KP_0
#define PR_KEY_KP_1						Prehistoric::InputCode::KEY_KP_1
#define PR_KEY_KP_2						Prehistoric::InputCode::KEY_KP_2
#define PR_KEY_KP_3						Prehistoric::InputCode::KEY_KP_3
#define PR_KEY_KP_4						Prehistoric::InputCode::KEY_KP_4
#define PR_KEY_KP_5						Prehistoric::InputCode::KEY_KP_5
#define PR_KEY_KP_6						Prehistoric::InputCode::KEY_KP_6
#define PR_KEY_KP_7						Prehistoric::InputCode::KEY_KP_7
#define PR_KEY_KP_8						Prehistoric::InputCode::KEY_KP_8
#define PR_KEY_KP_9						Prehistoric::InputCode::KEY_KP_9
#define PR_KEY_KP_DECIMAL				Prehistoric::InputCode::KEY_KP_DECIMAL
#define PR_KEY_KP_DIVIDE				Prehistoric::InputCode::KEY_KP_DIVIDE
#define PR_KEY_KP_MULTIPLY				Prehistoric::InputCode::KEY_KP_MULTIPLY
#define PR_KEY_KP_SUBTRACT				Prehistoric::InputCode::KEY_KP_SUBTRACT
#define PR_KEY_KP_ADD					Prehistoric::InputCode::KEY_KP_ADD
#define PR_KEY_KP_ENTER					Prehistoric::InputCode::KEY_KP_ENTER
#define PR_KEY_KP_EQUAL					Prehistoric::InputCode::KEY_KP_EQUAL
#define PR_KEY_LEFT_SHIFT				Prehistoric::InputCode::KEY_LEFT_SHIFT
#define PR_KEY_LEFT_CONTROL				Prehistoric::InputCode::KEY_LEFT_CONTROL
#define PR_KEY_LEFT_ALT					Prehistoric::InputCode::KEY_LEFT_ALT
#define PR_KEY_LEFT_SUPER				Prehistoric::InputCode::KEY_LEFT_SUPER
#define PR_KEY_RIGHT_SHIFT				Prehistoric::InputCode::KEY_RIGHT_SHIFT
#define PR_KEY_RIGHT_CONTROL			Prehistoric::InputCode::KEY_RIGHT_CONTROL
#define PR_KEY_RIGHT_ALT				Prehistoric::InputCode::KEY_RIGHT_ALT
#define PR_KEY_RIGHT_SUPER				Prehistoric::InputCode::KEY_RIGHT_SUPER
#define PR_KEY_RIGHT_WINDOWS			Prehistoric::InputCode::KEY_RIGHT_SUPER
#define PR_KEY_RIGHT_COMMAND			Prehistoric::InputCode::KEY_RIGHT_SUPER
#define PR_KEY_MENU						Prehistoric::InputCode::KEY_MENU

#define PR_MOUSE_BUTTON_1				Prehistoric::InputCode::MOUSE_BUTTON_1
#define PR_MOUSE_BUTTON_2				Prehistoric::InputCode::MOUSE_BUTTON_2
#define PR_MOUSE_BUTTON_3				Prehistoric::InputCode::MOUSE_BUTTON_3
#define PR_MOUSE_LEFT_BUTTON			Prehistoric::InputCode::MOUSE_BUTTON_1
#define PR_MOUSE_RIGHT_BUTTON			Prehistoric::InputCode::MOUSE_BUTTON_2
#define PR_MOUSE_MIDDLE_BUTTON			Prehistoric::InputCode::MOUSE_BUTTON_3
#define PR_MOUSE_BUTTON_4				Prehistoric::InputCode::MOUSE_BUTTON_4
#define PR_MOUSE_BUTTON_5				Prehistoric::InputCode::MOUSE_BUTTON_5
#define PR_MOUSE_BUTTON_6				Prehistoric::InputCode::MOUSE_BUTTON_6
#define PR_MOUSE_BUTTON_7				Prehistoric::InputCode::MOUSE_BUTTON_7
#define PR_MOUSE_BUTTON_8				Prehistoric::InputCode::MOUSE_BUTTON_8
#define PR_MOUSE_BUTTON_LAST			Prehistoric::InputCode::MOUSE_BUTTON_LAST
#define PR_MOUSE_BUTTON_LEFT			Prehistoric::InputCode::MOUSE_BUTTON_LEFT
#define PR_MOUSE_BUTTON_RIGHT			Prehistoric::InputCode::MOUSE_BUTTON_RIGHT
#define PR_MOUSE_BUTTON_MIDDLE			Prehistoric::InputCode::MOUSE_BUTTON_MIDDLE

#define PR_JOYSTICK_1					Prehistoric::JoystickID::JOYSTICK_1
#define PR_JOYSTICK_2					Prehistoric::JoystickID::JOYSTICK_2
#define PR_JOYSTICK_3					Prehistoric::JoystickID::JOYSTICK_3
#define PR_JOYSTICK_4					Prehistoric::JoystickID::JOYSTICK_4
#define PR_JOYSTICK_5					Prehistoric::JoystickID::JOYSTICK_5
#define PR_JOYSTICK_6					Prehistoric::JoystickID::JOYSTICK_6
#define PR_JOYSTICK_7					Prehistoric::JoystickID::JOYSTICK_7
#define PR_JOYSTICK_8					Prehistoric::JoystickID::JOYSTICK_8
#define PR_JOYSTICK_9					Prehistoric::JoystickID::JOYSTICK_9
#define PR_JOYSTICK_10					Prehistoric::JoystickID::JOYSTICK_10
#define PR_JOYSTICK_11					Prehistoric::JoystickID::JOYSTICK_11
#define PR_JOYSTICK_12					Prehistoric::JoystickID::JOYSTICK_12
#define PR_JOYSTICK_13					Prehistoric::JoystickID::JOYSTICK_13
#define PR_JOYSTICK_14					Prehistoric::JoystickID::JOYSTICK_14
#define PR_JOYSTICK_15					Prehistoric::JoystickID::JOYSTICK_15
#define PR_JOYSTICK_16					Prehistoric::JoystickID::JOYSTICK_16
#define PR_JOYSTICK_LAST				Prehistoric::JoystickID::JOYSTICK_LAST

#define PR_GAMEPAD_BUTTON_A				Prehistoric::InputCode::GAMEPAD_BUTTON_A
#define PR_GAMEPAD_BUTTON_B				Prehistoric::InputCode::GAMEPAD_BUTTON_B
#define PR_GAMEPAD_BUTTON_X				Prehistoric::InputCode::GAMEPAD_BUTTON_X
#define PR_GAMEPAD_BUTTON_Y				Prehistoric::InputCode::GAMEPAD_BUTTON_Y
#define PR_GAMEPAD_BUTTON_LEFT_BUMPER	Prehistoric::InputCode::GAMEPAD_BUTTON_LEFT_BUMPER
#define PR_GAMEPAD_BUTTON_RIGHT_BUMPER	Prehistoric::InputCode::GAMEPAD_BUTTON_RIGHT_BUMPER
#define PR_GAMEPAD_BUTTON_BACK			Prehistoric::InputCode::GAMEPAD_BUTTON_BACK
#define PR_GAMEPAD_BUTTON_START			Prehistoric::InputCode::GAMEPAD_BUTTON_START
#define PR_GAMEPAD_BUTTON_GUIDE			Prehistoric::InputCode::GAMEPAD_BUTTON_GUIDE
#define PR_GAMEPAD_BUTTON_LEFT_THUMB	Prehistoric::InputCode::GAMEPAD_BUTTON_LEFT_THUMB
#define PR_GAMEPAD_BUTTON_RIGHT_THUMB	Prehistoric::InputCode::GAMEPAD_BUTTON_RIGHT_THUMB
#define PR_GAMEPAD_BUTTON_DPAD_UP		Prehistoric::InputCode::GAMEPAD_BUTTON_DPAD_UP
#define PR_GAMEPAD_BUTTON_DPAD_RIGHT	Prehistoric::InputCode::GAMEPAD_BUTTON_DPAD_RIGHT
#define PR_GAMEPAD_BUTTON_DPAD_DOWN		Prehistoric::InputCode::GAMEPAD_BUTTON_DPAD_DOWN
#define PR_GAMEPAD_BUTTON_DPAD_LEFT		Prehistoric::InputCode::GAMEPAD_BUTTON_DPAD_LEFT
#define PR_GAMEPAD_BUTTON_LAST			Prehistoric::InputCode::GAMEPAD_BUTTON_LAST

#define PR_GAMEPAD_BUTTON_CROSS			Prehistoric::InputCode::GAMEPAD_BUTTON_CROSS
#define PR_GAMEPAD_BUTTON_CIRCLE		Prehistoric::InputCode::GAMEPAD_BUTTON_CIRCLE
#define PR_GAMEPAD_BUTTON_SQUARE		Prehistoric::InputCode::GAMEPAD_BUTTON_SQUARE
#define PR_GAMEPAD_BUTTON_TRIANGLE		Prehistoric::InputCode::GAMEPAD_BUTTON_TRIANGLE

#define PR_GAMEPAD_AXIS_LEFT_X			Prehistoric::InputCode::GAMEPAD_AXIS_LEFT_X
#define PR_GAMEPAD_AXIS_LEFT_Y			Prehistoric::InputCode::GAMEPAD_AXIS_LEFT_Y
#define PR_GAMEPAD_AXIS_RIGHT_X			Prehistoric::InputCode::GAMEPAD_AXIS_RIGHT_X
#define PR_GAMEPAD_AXIS_RIGHT_Y			Prehistoric::InputCode::GAMEPAD_AXIS_RIGHT_Y
#define PR_GAMEPAD_AXIS_LEFT_TRIGGER	Prehistoric::InputCode::GAMEPAD_AXIS_LEFT_TRIGGER
#define PR_GAMEPAD_AXIS_RIGHT_TRIGGER	Prehistoric::InputCode::GAMEPAD_AXIS_RIGHT_TRIGGER
#define PR_GAMEPAD_AXIS_LAST			Prehistoric::InputCode::GAMEPAD_AXIS_LAST

#endif