#ifndef GL_DEPTH_PASS_H
#define GL_DEPTH_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "prehistoric/common/rendering/Renderer.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	class GLDepthPass : public RenderStage
	{
	public:
		GLDepthPass(Renderer* renderer);
		virtual ~GLDepthPass();

		virtual void OnResized() override;
		virtual void Render() override;

		static void DrawBatches(const ModelBatches& batches, Shader& shader);

		TextureHandle getDepthTexture() const { return depthTexture; }
	private:
		std::unique_ptr<GLFramebuffer> framebuffer;
		ShaderHandle depthShader;
		TextureHandle depthTexture;
	};
};

#endif