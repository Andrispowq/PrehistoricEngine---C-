#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include "Event.h"
#include "prehistoric/core/util/codes/InputCode.h"

namespace Prehistoric
{
	class KeyEvent : public Event
	{
	public:
		InputCode getKeyCode() const { return keyCode; }
		void* getHandle() const { return handle; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(InputCode keycode, void* handle)
			: keyCode(keycode), handle(handle) {}

		InputCode keyCode;
		void* handle;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(InputCode keycode, void* handle, uint16_t repeatCount)
			: KeyEvent(keycode, handle), repeatCount(repeatCount) {}

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

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(InputCode keycode, void* handle)
			: KeyEvent(keycode, handle) {}

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