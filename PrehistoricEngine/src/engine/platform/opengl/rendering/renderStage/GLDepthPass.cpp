#include "Includes.hpp"
#include "GLDepthPass.h"

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	GLDepthPass::GLDepthPass(Renderer* renderer)
		: RenderStage(renderer), renderer(renderer)
	{
	}

	void GLDepthPass::Render()
	{
		GLRenderer* rend = (GLRenderer*)renderer;
	}
};
