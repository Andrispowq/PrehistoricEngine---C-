#ifndef GL_SHADOW_DEPTH_PASS_H
#define GL_SHADOW_DEPTH_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"
#include "platform/opengl/rendering/shaders/shadow/GLShadowDepthPassShader.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;
	class Light;

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
		static constexpr float CascadeBlendFraction = 0.1f;
		static const Light* FindSun(const Renderer& renderer);
		const Light* getSun() const { return rendered ? sun : nullptr; }
		const Matrix4f& getShadowView() const { return shadowView; }
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
		const Light* sun = nullptr;
		Matrix4f shadowView = Matrix4f::Identity();
		std::unique_ptr<UniformBufferObject> matrices;
	};
};

#endif
