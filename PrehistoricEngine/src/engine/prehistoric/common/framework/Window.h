#ifndef WINDOW_H
#define WINDOW_H

#include "prehistoric/core/util/Constants.h"

#include "prehistoric/common/framework/swapchain/Swapchain.h"
#include "prehistoric/common/framework/context/Context.h"

#include "prehistoric/core/config/FrameworkConfig.h"

#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	struct WindowData
	{
		uint32_t width;
		uint32_t height;

		std::unique_ptr<Swapchain> swapchain;
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window();
		virtual ~Window();

		virtual bool Create() = 0;
		virtual bool ShouldClose() const = 0;
		virtual void Input() = 0;
		virtual void Render(CommandBuffer* buffer) const = 0;

		virtual void OnEvent(Event& event) = 0;

		virtual void* getWindowHandle() const = 0;
		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual bool IsFullscreen() const = 0;

		void ClearScreen() const { swapchain->ClearScreen(); }
		void SetVSync(bool vsync) const { swapchain->SetVSync(vsync); }

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		bool getClosed() const { return closed; }

		Swapchain* getSwapchain() const { return swapchain.get(); }
		Context* getContext() const { return context.get(); }

		void setWidth(uint32_t width) { this->width = width; FrameworkConfig::windowWidth = width; }
		void setHeight(uint32_t height) { this->height = height; FrameworkConfig::windowHeight = height; }
		void setClosed(const bool& closed) { this->closed = closed; }
		void setClearColour(const Vector4f& clearColour) { swapchain->setClearColour(clearColour); }

		void setEventCallback(EventCallbackFn function) { this->eventCallback = function; }
		EventCallbackFn getEventCallback() const { return eventCallback; }

	protected:
		uint32_t width;
		uint32_t height;

		const char* title;
		bool fullscreen;
		bool closed;

		std::unique_ptr<Context> context;
		std::unique_ptr<Swapchain> swapchain;

		EventCallbackFn eventCallback;
	};
};

#endif