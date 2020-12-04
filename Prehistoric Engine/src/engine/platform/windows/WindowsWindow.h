#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

#include "engine/prehistoric/common/framework/Window.h"

#include <glew.h>
#include <glfw3.h>

#include "engine/prehistoric/core/util/loader/TextureLoader.h"
#include "engine/config/FrameworkConfig.h"
#include "engine/platform/Platform.h"

class WindowsWindow : public Window
{
public:
	WindowsWindow() : Window(), window(nullptr) {}

	virtual ~WindowsWindow();

	virtual bool Create() override;
	virtual bool ShouldClose() const override;
	virtual void Input() override;
	virtual void Render(CommandBuffer* buffer) const override;

	virtual void SetFullscreen(bool fullscreen) override;
	virtual bool IsFullscreen() const override;

	virtual void* getWindowHandle() const override { return window; }
private:
	bool initGLFW() const;
private:
	int oldWidth, oldHeight;
	int oldX, oldY;

	GLFWwindow* window;
};

#endif
