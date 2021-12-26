#include "Includes.hpp"
#include "GLShadowDepthPass.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"

namespace Prehistoric
{
	Texture* shadowTex = nullptr;
	Matrix4f toLightSpace;

	GLShadowDepthPass::GLShadowDepthPass(Renderer* renderer)
		: RenderStage(renderer), framebuffer{ nullptr }
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		float aspect = float(width) / float(height);

		farWidth = (float)(SHADOW_DISTANCE * tan(ToRadians(camera->getFov())));
		nearWidth = (float)(EngineConfig::rendererNearPlane * tan(ToRadians(camera->getFov())));
		farHeight = farWidth / aspect;
		nearHeight = nearWidth / aspect;

		lightViewMatrix = Matrix4f::Identity();
		lightProjMatrix = Matrix4f::Identity();

		AssetManager* man = manager->getAssetManager();
		depthTexture = man->storeTexture(GLTexture::Storage2D(SIZE, SIZE, 1, D32_LINEAR, Nearest, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		man->addReference<Shader>(depthShader.handle);

		framebuffer = std::make_unique<GLFramebuffer>(window);
		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	GLShadowDepthPass::~GLShadowDepthPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Shader>(depthShader.handle);
		man->removeReference<Texture>(depthTexture.handle);
	}

	void GLShadowDepthPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(depthTexture.handle);
		depthTexture = man->storeTexture(GLTexture::Storage2D(SIZE, SIZE, 1, D32_LINEAR, Nearest, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	void GLShadowDepthPass::Render()
	{
		PR_PROFILE("Depth pass");
		GLRenderer* rend = (GLRenderer*)renderer;
		
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		//update stuff
		CalculateProjection();

		lightProjMatrix = Matrix4f::Identity();
		lightProjMatrix.m[0 * 4 + 0] = 2.f / (maxX - minX);
		lightProjMatrix.m[1 * 4 + 1] = 2.f / (maxY - minY);
		lightProjMatrix.m[2 * 4 + 2] = -2.f / (maxZ - minZ);
		lightProjMatrix.m[3 * 4 + 3] = 1;
		CalculateLightViewMatrix(Vector3f(0, -1, 1), getBoxCentre());

		/// <summary>
		/// EXPERIMENTAL
		/// </summary>
		shadowTex = depthTexture.pointer;
		lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
		lightSpaceMatrix = Matrix4f::Transformation(0.5f, 0.0f, 0.5f) * lightSpaceMatrix;
		toLightSpace = lightSpaceMatrix;
		/// <summary>
		/// EXPERIMENTAL
		/// </summary>

		framebuffer->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		framebuffer->SetDrawAttachments(1, arr);
		framebuffer->Clear(0.0f);

		window->getSwapchain()->SetWindowSize(SIZE, SIZE);

		depthShader->Bind(nullptr);
		dynamic_cast<GLDepthPassShader*>(depthShader.pointer)->UpdateGlobalUniforms(lightProjMatrix, lightViewMatrix);

		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;
			VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
			vbo->Bind(nullptr);

			for (auto material : pipeline.second)
			{
				for (auto renderer_ : material.second)
				{
					depthShader->UpdateObjectUniforms(renderer_->getParent());

					for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
					{
						vbo->Draw(nullptr, i);
					}
				}
			}

			static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
		}

		//TODO: enable alpha blending
		for (auto pipeline : rend->getModelsTransparency())
		{
			Pipeline* pl = pipeline.first;
			VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
			vbo->Bind(nullptr);

			for (auto material : pipeline.second)
			{
				for (auto renderer_ : material.second)
				{
					depthShader->UpdateObjectUniforms(renderer_->getParent());

					for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
					{
						vbo->Draw(nullptr, i);
					}
				}
			}

			static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
		}

		window->getSwapchain()->SetWindowSize(width, height);

		depthShader->Unbind();
		framebuffer->Unbind();
	}

	void GLShadowDepthPass::CalculateProjection()
	{
		Matrix4f rotation = Matrix4f::View(camera->getForward(), camera->getUp());
		Vector3f forward = (rotation * Vector4f(0, 0, -1, 0)).xyz();

		Vector3f toFar = forward * SHADOW_DISTANCE;
		Vector3f toNear = forward * NEAR_PLANE;
		Vector3f centerFar = toFar + camera->getPosition();
		Vector3f centerNear = toNear + camera->getPosition();

		std::array<Vector4f, 8> points = CalculateFrustumVertices(rotation, forward, centerNear, centerFar);

		boolean first = true;
		for (Vector4f point : points) 
		{
			if (first)
			{
				minX = point.x;
				maxX = point.x;
				minY = point.y;
				maxY = point.y;
				minZ = point.z;
				maxZ = point.z;
				first = false;
				continue;
			}
			if (point.x > maxX) 
			{
				maxX = point.x;
			}
			else if (point.x < minX) 
			{
				minX = point.x;
			}
			if (point.y > maxY)
			{
				maxY = point.y;
			}
			else if (point.y < minY) 
			{
				minY = point.y;
			}
			if (point.z > maxZ)
			{
				maxZ = point.z;
			}
			else if (point.z < minZ) 
			{
				minZ = point.z;
			}
		}

		maxZ += OFFSET;
	}

	std::array<Vector4f, 8> GLShadowDepthPass::CalculateFrustumVertices(Matrix4f rotation, Vector3f forwardVector, Vector3f centerNear, Vector3f centerFar)
	{
		std::array<Vector4f, 8> points;

		Vector3f upVector = (rotation * Vector4f(camera->getUp(), 0)).xyz();
		Vector3f rightVector = forwardVector.cross(upVector);
		Vector3f downVector = upVector * -1;
		Vector3f leftVector = rightVector * -1;
		Vector3f farTop = centerFar + upVector * farHeight;
		Vector3f farBottom = centerFar + downVector * farHeight;
		Vector3f nearTop = centerNear + upVector * nearHeight;
		Vector3f nearBottom = centerNear + downVector * nearHeight;

		points[0] = CalculateLightSpaceFrustumCorner(farTop, rightVector, farWidth);
		points[1] = CalculateLightSpaceFrustumCorner(farTop, leftVector, farWidth);
		points[2] = CalculateLightSpaceFrustumCorner(farBottom, rightVector, farWidth);
		points[3] = CalculateLightSpaceFrustumCorner(farBottom, leftVector, farWidth);
		points[4] = CalculateLightSpaceFrustumCorner(nearTop, rightVector, nearWidth);
		points[5] = CalculateLightSpaceFrustumCorner(nearTop, leftVector, nearWidth);
		points[6] = CalculateLightSpaceFrustumCorner(nearBottom, rightVector, nearWidth);
		points[7] = CalculateLightSpaceFrustumCorner(nearBottom, leftVector, nearWidth);

		return points;
	}

	Vector4f GLShadowDepthPass::CalculateLightSpaceFrustumCorner(Vector3f startPoint, Vector3f direction, float width)
	{
		Vector3f point = startPoint + direction * width;
		Vector4f point4f = Vector4f(point, 1);
		point4f = lightViewMatrix * point4f;
		return point4f;
	}

	void GLShadowDepthPass::CalculateLightViewMatrix(Vector3f direction, Vector3f center)
	{
		direction = direction.normalise();
		center = center.negate();
		lightViewMatrix = Matrix4f::Identity();
		float pitch = (float)ToDegrees(acos(Vector2f(direction.x, direction.z).length()));
		float yaw = (float)ToDegrees(((float)atan(direction.x / direction.z)));
		yaw = direction.z > 0 ? yaw - 180 : yaw;
		lightViewMatrix = Matrix4f::Rotation({ pitch, -yaw, 2 }) * Matrix4f::Translation(center);

		/*float pitch = (float)Math.acos(new Vector2f(direction.x, direction.z).length());
		Matrix4f.rotate(pitch, new Vector3f(1, 0, 0), lightViewMatrix, lightViewMatrix);
		float yaw = (float)Math.toDegrees(((float)Math.atan(direction.x / direction.z)));
		yaw = direction.z > 0 ? yaw - 180 : yaw;
		Matrix4f.rotate((float)-Math.toRadians(yaw), new Vector3f(0, 1, 0), lightViewMatrix, lightViewMatrix);
		Matrix4f.translate(center, lightViewMatrix, lightViewMatrix);*/
	}

	Vector3f GLShadowDepthPass::getBoxCentre() const
	{
		float x = (minX + maxX) / 2;
		float y = (minY + maxY) / 2;
		float z = (minZ + maxZ) / 2;
		Vector4f cen = Vector4f(x, y, z, 1);
		Matrix4f invertedLight = lightViewMatrix.Invert();
		return (invertedLight * cen).xyz();
	}
};
