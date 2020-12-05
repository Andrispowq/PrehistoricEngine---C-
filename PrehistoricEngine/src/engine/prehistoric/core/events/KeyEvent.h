#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include "Event.h"
#include "prehistoric/core/util/codes/InputCode.h"

namespace Prehistoric
{
	class PR_API KeyEvent : public Event
	{
	public:
		InputCode getKeyCode() const { return keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const InputCode keycode)
			: keyCode(keycode) {}

		InputCode keyCode;
	};

	class PR_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const InputCode keycode, const uint16_t repeatCount)
			: KeyEvent(keycode), repeatCount(repeatCount) {}

		uint16_t getRepeatCount() const { return repeatCount; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << (uint16_t)keyCode << " (" << repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		uint16_t repeatCount;
	};

	class PR_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const InputCode keycode)
			: KeyEvent(keycode) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << (uint16_t)keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
};

#endif