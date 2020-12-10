#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "Event.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(Vector2f mousePos)
			: mousePosition(mousePos) {}

		float getX() const { return mousePosition.x; }
		float getY() const { return mousePosition.y; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << mousePosition.x << ", " << mousePosition.y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		Vector2f mousePosition;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(Vector2f mouseOffset)
			: mouseOffset(mouseOffset) {}

		float getXOffset() const { return mouseOffset.x; }
		float getYOffset() const { return mouseOffset.y; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << mouseOffset.x << ", " << mouseOffset.y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		Vector2f mouseOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		InputCode GetMouseButton() const { return button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		MouseButtonEvent(const InputCode button)
			: button(button) {}

		InputCode button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const InputCode button)
			: MouseButtonEvent(button) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << (uint16_t)button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const InputCode button)
			: MouseButtonEvent(button) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << (uint16_t)button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
};

#endif