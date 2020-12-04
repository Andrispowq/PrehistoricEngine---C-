#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "engine/prehistoric/common/framework/Window.h"

class RenderUtils
{
public:
	RenderUtils(Window* window) : window(window) {}
	virtual ~RenderUtils() = 0;
	
	virtual void setAlphaBlending(bool alphaBlending) = 0;
	virtual void setDepthTesting(bool depthTesting) = 0;

private:
	Window* window;

	static RenderUtils* instance;
};

#endif