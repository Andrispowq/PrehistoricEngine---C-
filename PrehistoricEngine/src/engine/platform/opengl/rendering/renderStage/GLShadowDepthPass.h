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

		// Shadow resolution is independent of the window extent.
		void OnResized() override {}
		virtual void Render() override;

		static constexpr size_t CascadeCount = 6;
		const std::array<float, CascadeCount>& getCascadeDistances() const { return shadowCascadeLevels; }
		void BindMatrices() const;
		bool hasShadowMap() const { return rendered; }
		TextureHandle getDepthTexture() const { return depthTexture; }
	private:
		std::vector<Vector4f> GetFrustumCornersWorldSpace(Matrix4f proj, Matrix4f view);
		Matrix4f GetViewProjMatrix(Vector3f lightDir, float nearPlane, float farPlane);
		std::vector<Matrix4f> GetLightSpaceMatrices(Vector3f lightDir);

	private:
		std::unique_ptr<GLFramebuffer> framebuffer;
		ShaderHandle depthShader;
		TextureHandle depthTexture;

		std::array<float, CascadeCount> shadowCascadeLevels;
		bool rendered = false;
		std::unique_ptr<UniformBufferObject> matrices;
	};
};

#endif