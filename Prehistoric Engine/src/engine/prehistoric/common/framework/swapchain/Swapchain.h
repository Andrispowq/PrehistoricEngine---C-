#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "engine/prehistoric/core/math/Math.h"

#include "engine/prehistoric/common/rendering/command/CommandBuffer.h"

class Window;

class Swapchain
{
public:
	Swapchain(Window* window) : window(window) {}
	virtual ~Swapchain() = 0;

	virtual void SwapBuffers(CommandBuffer* buffer) = 0;
	virtual void ClearScreen() = 0;

	virtual void SetVSync(bool vSync) const = 0;
	virtual void SetWindowSize(uint32_t width, uint32_t height) = 0;

	virtual uint32_t getAquiredImageIndex() const { return -1; };

	Vector4f getClearColour() const { return clearColour; }
	void setClearColour(const Vector4f& colour) { this->clearColour = colour; }
protected:
	Window* window;

	Vector4f clearColour;
};

#endif