#ifndef GL_RAY_TRACING_PASS_H
#define GL_RAY_TRACING_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLRayTracingPipeline.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLRayTracingPass : public RenderStage
	{
	public:
		GLRayTracingPass(Renderer* renderer);
		virtual ~GLRayTracingPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getOutputImage() const { return outputImage; }

	private:
		ShaderHandle rayTracingShader;
		PipelineHandle rayTracingPipeline;
		uint32_t current_spp = 1;

		TextureHandle outputImage;
	};
};

#endif