#ifndef APPLICATION_EVENT_H
#define APPLICATION_EVENT_H

#include "Event.h"

namespace Prehistoric 
{
	class WindowEvent : public Event
	{
	public:
		void* getHandle() const { return handle; }
	protected:
		WindowEvent(void* handle)
			: handle(handle) {}

		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	protected:
		void* handle;
	};

	class WindowResizeEvent : public WindowEvent
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height, void* handle)
			: WindowEvent(handle), width(width), height(height) {}

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << width << ", " << height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
	private:
		uint32_t width, height;
		void* handle;
	};

	class WindowCloseEvent : public WindowEvent
	{
	public:
		WindowCloseEvent(void* handle) : WindowEvent(handle) {}

		EVENT_CLASS_TYPE(WindowClose)
	};

	class WindowFocusEvent : public WindowEvent
	{
	public:
		WindowFocusEvent(void* handle) : WindowEvent(handle) {}

		EVENT_CLASS_TYPE(WindowFocus)
	};

	class WindowLostFocusEvent : public WindowEvent
	{
	public:
		WindowLostFocusEvent(void* handle) : WindowEvent(handle) {}

		EVENT_CLASS_TYPE(WindowLostFocus)
	};

	class WindowIconifiedEvent : public WindowEvent
	{
	public:
		WindowIconifiedEvent(void* handle) : WindowEvent(handle) {}

		EVENT_CLASS_TYPE(WindowIconified)
	};

	class WindowRestoredEvent : public WindowEvent
	{
	public:
		WindowRestoredEvent(void* handle) : WindowEvent(handle) {}

		EVENT_CLASS_TYPE(WindowRestored)
	};

	class AppInputEvent : public Event
	{
	public:
		AppInputEvent() = default;

		EVENT_CLASS_TYPE(AppInput)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
};

#endif