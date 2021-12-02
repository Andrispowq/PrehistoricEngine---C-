#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "prehistoric/common/rendering/Renderer.h"

#include <glad/glad.h>

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/renderStage/GLDepthPass.h"
#include "platform/opengl/rendering/renderStage/GLLightCullingPass.h"
#include "platform/opengl/rendering/renderStage/GLMainPass.h"
#include "platform/opengl/rendering/renderStage/GLBloomPass.h"
#include "platform/opengl/rendering/renderStage/GLHDRPass.h"

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

		virtual Texture* getOutputTexture() const override { return hdrPass->getOutputImage().pointer; }

		GLDepthPass* getDepthPass() const { return depthPass; }
		GLLightCullingPass* getLightCullingPass() const { return lightCullingPass; }
		GLMainPass* getMainPass() const { return mainPass; }
		GLBloomPass* getBloomPass() const { return bloomPass; }
		GLHDRPass* getHDRPass() const { return hdrPass; }

	protected:
		GLDepthPass* depthPass;
		GLLightCullingPass* lightCullingPass;
		GLMainPass* mainPass;
		GLBloomPass* bloomPass;
		GLHDRPass* hdrPass;

		VertexBufferHandle quad;
		ShaderHandle renderShader;
		PipelineHandle renderPipeline;
	};
};

#endif