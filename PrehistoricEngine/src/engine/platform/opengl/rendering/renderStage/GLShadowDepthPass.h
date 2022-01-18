#ifndef GL_SHADOW_DEPTH_PASS_H
#define GL_SHADOW_DEPTH_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/shaders/shadow/GLShadowDepthPassShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	constexpr float SIZE = 4096;

	class GLShadowDepthPass : public RenderStage
	{
	public:
		GLShadowDepthPass(Renderer* renderer);
		virtual ~GLShadowDepthPass();

		virtual void OnResized() override;
		virtual void Render() override;

		TextureHandle getDepthTexture() const { return depthTexture; }
	private:
		std::vector<Vector4f> GetFrustumCornersWorldSpace(Matrix4f proj, Matrix4f view);
		Matrix4f GetViewProjMatrix(Vector3f lightDir, float nearPlane, float farPlane);
		std::vector<Matrix4f> GetLightSpaceMatrices(Vector3f lightDir);

	private:
		std::unique_ptr<GLFramebuffer> framebuffer;
		ShaderHandle depthShader;
		TextureHandle depthTexture;

		std::vector<float> shadowCascadeLevels;
		UniformBufferObject* matrices;
	};
};

#endif