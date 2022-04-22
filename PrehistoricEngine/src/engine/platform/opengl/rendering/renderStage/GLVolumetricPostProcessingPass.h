#ifndef GL_VOLUMETRIC_POST_PROCESSING_PASS_H
#define GL_VOLUMETRIC_POST_PROCESSING_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "platform/opengl/rendering/shaders/postProcessing/GLVolumetricPostProcessingShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLVolumetricPostProcessingPass : public RenderStage
	{
	public:
		GLVolumetricPostProcessingPass(Renderer* renderer);
		virtual ~GLVolumetricPostProcessingPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getOutputTexture() const { return outputImage; }

	private:
		ShaderHandle shader;
		PipelineHandle renderPipeline;

		TextureHandle outputImage;

		std::vector<float> shadowCascadeLevels;
	};
};

#endif