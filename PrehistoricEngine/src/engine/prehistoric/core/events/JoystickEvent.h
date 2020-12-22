#ifndef JOYSTICK_EVENT_H
#define JOYSTICK_EVENT_H

#include "Event.h"
#include "prehistoric/core/util/codes/InputCode.h"

namespace Prehistoric
{
	class PR_API JoystickEvent : public Event
	{
	public:
		JoystickID getJoystickID() const { return id; }

		EVENT_CLASS_CATEGORY(EventCategoryJoystick | EventCategoryInput)
	protected:
		JoystickEvent(JoystickID id)
			: id(id) {}

		JoystickID id;
	};

	class PR_API JoystickConnectedEvent : public JoystickEvent
	{
	public:
		JoystickConnectedEvent(JoystickID id)
			: JoystickEvent(id) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "JoystickConntectedEvent: " << (uint16_t)id;
			return ss.str();
		}

		EVENT_CLASS_TYPE(JoystickConnected)
	};

	class PR_API JoystickDisconnectedEvent : public JoystickEvent
	{
	public:
		JoystickDisconnectedEvent(JoystickID id)
			: JoystickEvent(id) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "JoystickDisconntectedEvent: " << (uint16_t)id;
			return ss.str();
		}

		EVENT_CLASS_TYPE(JoystickDisconnected)
	};
};

#endif