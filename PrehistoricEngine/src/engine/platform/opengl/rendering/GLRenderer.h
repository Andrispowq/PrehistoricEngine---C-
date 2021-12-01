#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glad/glad.h>

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/renderStage/GLDepthPass.h"
#include "platform/opengl/rendering/renderStage/GLLightCullingPass.h"

namespace Prehistoric
{
#define BLOOM_PASSES 8

	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~GLRenderer();

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;

		virtual Texture* getOutputTexture() const override { return outputImage.pointer; }

		GLDepthPass* getDepthPass() const { return depthPass; }
		GLLightCullingPass* getLightCullingPass() const { return lightCullingPass; }

	private:
		void UpdateLightBuffer();

	protected:
		GLDepthPass* depthPass;
		GLLightCullingPass* lightCullingPass;

		std::unique_ptr<GLFramebuffer> multisampleFBO;
		std::unique_ptr<GLFramebuffer> colourFBO;

		std::unique_ptr<GLFramebuffer> scratchFBO;

		TextureHandle colourImage;
		TextureHandle bloomImage;
		TextureHandle combinedImage;
		TextureHandle outputImage;

		TextureHandle temporaryImages[BLOOM_PASSES];
		TextureHandle bloomImages[BLOOM_PASSES];

		ShaderHandle decomposeShader;
		ShaderHandle gaussianShader;
		ShaderHandle bloomCombineShader;
		ShaderHandle hdrShader;
		ShaderHandle renderShader;

		VertexBufferHandle quad;

		PipelineHandle decomposePipeline;
		PipelineHandle gaussianPipeline;
		PipelineHandle bloomCombinePipeline;
		PipelineHandle hdrPipeline;
		PipelineHandle renderPipeline;
	};
};

#endif