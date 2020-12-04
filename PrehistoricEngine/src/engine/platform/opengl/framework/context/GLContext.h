#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include <glew.h>
#include <glfw3.h>

#include "engine/prehistoric/common/framework/context/Context.h"
#include "engine/prehistoric/common/framework/Window.h"

class GLContext : public Context
{
public:
	GLContext(Window* window);
	virtual ~GLContext() {}
};

#endif