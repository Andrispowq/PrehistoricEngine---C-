#ifndef GL_MAIN_PASS_H
#define GL_MAIN_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLMainPass : public RenderStage
	{
	public:
		GLMainPass(Renderer* renderer);
		virtual ~GLMainPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getColourImage() const { return colourImage; }
		TextureHandle getBloomImage() const { return bloomImage; }

	private:
		std::unique_ptr<GLFramebuffer> multisampleFBO;
		std::unique_ptr<GLFramebuffer> colourFBO;

		TextureHandle colourImage;
		TextureHandle bloomImage;
	};
};

#endif