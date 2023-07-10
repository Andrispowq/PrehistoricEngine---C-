#ifndef GL_VOXEL_RENDERER_PASS_H
#define GL_VOXEL_RENDERER_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/buffer/GLShaderStorageBuffer.h"

namespace Prehistoric
{
	class GLVoxelRendererPass : public RenderStage
	{
	public:
		GLVoxelRendererPass(Renderer* renderer);
		virtual ~GLVoxelRendererPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getVoxelImage() const { return voxelImage; }
		TextureHandle getOutputImage() const { return outputImage; }

	private:
		const static int voxel_size = 64;

		std::unique_ptr<GLFramebuffer> voxel_fbo;
		std::unique_ptr<GLFramebuffer> fbo;

		TextureHandle voxelImage;
		TextureHandle outputImage;

		ShaderHandle voxelisationShader;
		ShaderHandle coneTracingShader;
	};
}

#endif