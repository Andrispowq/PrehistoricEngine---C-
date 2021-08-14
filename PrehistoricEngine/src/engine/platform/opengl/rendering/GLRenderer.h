#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glad/glad.h>

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

namespace Prehistoric
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~GLRenderer();

		virtual void PrepareRendering();
		virtual void EndRendering();

		virtual void Render() override;

		virtual Texture* getOutputTexture() const override { return outputImage; }

	private:
		void UpdateLightBuffer();

	private:
		std::unique_ptr<GLFramebuffer> depthFBO;
		std::unique_ptr<GLFramebuffer> multisampleFBO;
		std::unique_ptr<GLFramebuffer> colourFBO;

		std::unique_ptr<GLFramebuffer> scratchFBO;

		std::unique_ptr<GLShaderStorageBuffer> lightBuffer;
		std::unique_ptr<GLShaderStorageBuffer> visibleLightIndicesBuffer;

		Texture* depthImage;
		Texture* colourImage;
		Texture* bloomImage;
		Texture* combinedImage;
		Texture* outputImage;

		Texture* temporaryImages[8];
		Texture* bloomImages[8];

		ShaderHandle depthShader;
		ShaderHandle lightCullingShader;
		ShaderHandle decomposeShader;
		ShaderHandle gaussianShader;
		ShaderHandle bloomCombineShader;
		ShaderHandle hdrShader;
		ShaderHandle renderShader;

		VertexBufferHandle quad;

		Pipeline* lightCullingPipeline;
		Pipeline* decomposePipeline;
		Pipeline* gaussianPipeline;
		Pipeline* bloomCombinePipeline;
		Pipeline* hdrPipeline;
		Pipeline* renderPipeline;
	};
};

#endif