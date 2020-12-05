#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glew.h>

namespace Prehistoric
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera) : Renderer(window, camera) {}
		virtual ~GLRenderer() {}

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;
	};
};

#endif