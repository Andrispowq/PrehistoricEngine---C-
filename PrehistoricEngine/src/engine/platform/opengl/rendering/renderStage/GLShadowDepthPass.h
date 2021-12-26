#ifndef GL_SHADOW_DEPTH_PASS_H
#define GL_SHADOW_DEPTH_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"

#include "platform/opengl/rendering/framebuffer/GLFramebuffer.h"

namespace Prehistoric
{
	class AssembledAssetManager;
	class Renderer;

	constexpr float SHADOW_DISTANCE = 300;
	constexpr float NEAR_PLANE = 0.1f;
	constexpr float OFFSET = 15;
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
		void CalculateProjection();

		std::array<Vector4f, 8> CalculateFrustumVertices(Matrix4f rotation, Vector3f forwardVector, Vector3f centreNear,
			Vector3f centreFar);
		Vector4f CalculateLightSpaceFrustumCorner(Vector3f startPoint, Vector3f direction, float width);

		void CalculateLightViewMatrix(Vector3f direction, Vector3f centre);

		Vector3f getBoxCentre() const;

	private:
		std::unique_ptr<GLFramebuffer> framebuffer;
		ShaderHandle depthShader;
		TextureHandle depthTexture;

		float farHeight, nearHeight, farWidth, nearWidth;
		float minX = -1, minY = -1, minZ = -1, maxX = 1, maxY = 1, maxZ = 1;

		Matrix4f lightViewMatrix;
		Matrix4f lightProjMatrix;
		Matrix4f lightSpaceMatrix;
	};
};

#endif