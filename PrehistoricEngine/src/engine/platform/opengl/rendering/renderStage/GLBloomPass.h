#ifndef GL_BLOOM_PASS_H
#define GL_BLOOM_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#define BLOOM_PASSES 8

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLBloomPass : public RenderStage
	{
	public:
		GLBloomPass(Renderer* renderer);
		virtual ~GLBloomPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getCombinedImage() const { return combinedImage; }

	private:
		std::unique_ptr<GLFramebuffer> scratchFBO;

		TextureHandle combinedImage;
		TextureHandle temporaryImages[BLOOM_PASSES];
		TextureHandle bloomImages[BLOOM_PASSES];

		VertexBufferHandle quad;

		ShaderHandle decomposeShader;
		ShaderHandle gaussianShader;
		ShaderHandle bloomCombineShader;
		ShaderHandle renderShader;

		PipelineHandle decomposePipeline;
		PipelineHandle gaussianPipeline;
		PipelineHandle bloomCombinePipeline;
		PipelineHandle renderPipeline;
	};
};

#endif