#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include "prehistoric/common/framework/context/Context.h"

#include <glew.h>
#include <GLFW/glfw3.h>

#include "prehistoric/common/framework/Window.h"

namespace Prehistoric
{
	class GLContext : public Context
	{
	public:
		GLContext(Window* window);
		virtual ~GLContext() {}
	};
};

#endif