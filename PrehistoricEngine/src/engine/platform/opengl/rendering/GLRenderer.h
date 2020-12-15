#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glew.h>

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager) : Renderer(window, camera), manager(manager) {}
		virtual ~GLRenderer() {}

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;
	private:
		AssembledAssetManager* manager;
	};
};

#endif