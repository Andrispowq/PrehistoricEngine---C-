#ifndef GL_HDR_PASS_H
#define GL_HDR_PASS_H

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

	class GLHDRPass : public RenderStage
	{
	public:
		GLHDRPass(Renderer* renderer);
		virtual ~GLHDRPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getOutputImage() const { return outputImage; }

	private:
		TextureHandle outputImage;

		ShaderHandle hdrShader;
		PipelineHandle hdrPipeline;
	};
};

#endif