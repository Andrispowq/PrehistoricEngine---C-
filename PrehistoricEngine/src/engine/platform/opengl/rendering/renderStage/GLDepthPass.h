#ifndef GL_DEPTH_PASS_H
#define GL_DEPTH_PASS_H

#include "prehistoric/common/rendering//renderStage/RenderStage.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "platform/opengl/rendering/GLRenderer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLDepthPass : public RenderStage
	{
	public:
		GLDepthPass(Renderer* renderer);
		virtual ~GLDepthPass() {}

		virtual void Render() override;
	private:
		Renderer* renderer;
	private:
		friend class GLRenderer;
	};
};

#endif