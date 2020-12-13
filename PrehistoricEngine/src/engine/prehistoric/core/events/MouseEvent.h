#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "Event.h"

#include "prehistoric/core/util/math/Math.h"

namespace Prehistoric
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(Vector2f mousePos, void* handle)
			: mousePosition(mousePos), handle(handle) {}

		Vector2f getPosition() const { return mousePosition; }
		void* getHandle() const { return handle; }

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
		void* handle;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(Vector2f mouseOffset, void* handle)
			: mouseOffset(mouseOffset), handle(handle) {}

		Vector2f getOffset() const { return mouseOffset; }
		void* getHandle() const { return handle; }

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
		void* handle;
	};

	class MouseButtonEvent : public Event
	{
	public:
		InputCode getMouseButton() const { return button; }
		void* getHandle() const { return handle; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		MouseButtonEvent(InputCode button, void* handle)
			: button(button), handle(handle) {}

		InputCode button;
		void* handle;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(InputCode button, void* handle, uint16_t repeatCount)
			: MouseButtonEvent(button, handle), repeatCount(repeatCount) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << (uint16_t)button << " (" << repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	private:
		uint16_t repeatCount;
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(InputCode button, void* handle)
			: MouseButtonEvent(button, handle) {}

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