#include "Includes.hpp"
#include "GLShadowDepthPass.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/buffer/GLUniformBufferObject.h"

#include "prehistoric/application/Application.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Prehistoric
{
	GLShadowDepthPass::GLShadowDepthPass(Renderer* renderer)
		: RenderStage(renderer), framebuffer{ nullptr }
	{
		float farPlane = __EngineConfig.rendererFarPlane;
		const float nearPlane = __EngineConfig.rendererNearPlane;
		const float fractions[] = { 0.004f, 0.01f, 0.02f, 0.04f, 0.1f, 0.5f };
		for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
			shadowCascadeLevels[i] = nearPlane + (farPlane - nearPlane) * fractions[i];

		matrices = std::make_unique<GLUniformBufferObject>(window, nullptr, 16 * 16 * sizeof(float));

		AssetManager* man = manager->getAssetManager();
		depthTexture = man->storeTexture(GLTexture::Storage2DArray((uint32_t)SIZE, (uint32_t)SIZE, (int)shadowCascadeLevels.size() + 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		depthShader = man->loadShader(ShaderName::ShadowDepthPass).value();
		man->addReference<Shader>(depthShader.handle);

		framebuffer = std::make_unique<GLFramebuffer>(window);
		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	GLShadowDepthPass::~GLShadowDepthPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Shader>(depthShader.handle);
		man->removeReference<Texture>(depthTexture.handle);
	}

	void GLShadowDepthPass::BindMatrices() const
	{
		matrices->BindBase(nullptr, 0);
	}

	const Light* GLShadowDepthPass::FindSun(const Renderer& renderer)
	{
		for (const auto* light : renderer.getLights())
		{
			if (light->castShadows() && light->getParent()->getWorldTransform().getPosition().lengthSquared() > 0.0f)
				return light;
		}
		return nullptr;
	}

	void GLShadowDepthPass::Render()
	{
		PR_PROFILE("Depth pass");
		GLRenderer* rend = (GLRenderer*)renderer;
		
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		sun = FindSun(*renderer);

		if (sun == nullptr)
		{
			rendered = false;
			return;
		}

		//update stuff
		Vector3f sunPos = sun->getParent()->getWorldTransform().getPosition();
		shadowView = renderer->getCamera()->getViewMatrix();
		std::vector<Matrix4f> lightMatrices = GetLightSpaceMatrices(sunPos.normalise());


		matrices->Bind(0);
		matrices->MapBuffer();
		uint8_t* ptr = (uint8_t*)matrices->getMappedData();
		for (const auto& elem : lightMatrices)
		{
			memcpy(ptr, elem.m, 16 * sizeof(float));
			ptr += (16 * sizeof(float));
		}
		matrices->UnmapBuffer();
		matrices->Unbind();

		framebuffer->Bind();
		framebuffer->Clear(0.0f);

		window->getSwapchain()->SetWindowSize((uint32_t)SIZE, (uint32_t)SIZE);

		depthShader->Bind(nullptr);
		dynamic_cast<GLShadowDepthPassShader*>(depthShader.pointer)->UpdateGlobalUniforms(matrices.get());

		// Use mesh geometry for both opaque and transparent casters. The depth
		// shader currently treats transparent materials as solid silhouettes.
		const bool cullingEnabled = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);
		GLDepthPass::DrawBatches(rend->getModels3D(), *depthShader.pointer);
		GLDepthPass::DrawBatches(rend->getModelsTransparency(), *depthShader.pointer);
		if (cullingEnabled) glEnable(GL_CULL_FACE);

		window->getSwapchain()->SetWindowSize(width, height);

		rendered = true;
		depthShader->Unbind();
		framebuffer->Unbind();
	}

	std::vector<Vector4f> GLShadowDepthPass::GetFrustumCornersWorldSpace(Matrix4f proj, Matrix4f view)
	{
		Matrix4f inv = (proj * view).Invert();

		std::vector<Vector4f> frustumCorners;

		for (uint32_t x = 0; x < 2; ++x)
		{
			for (uint32_t y = 0; y < 2; ++y)
			{
				for (uint32_t z = 0; z < 2; ++z)
				{
					Vector4f pt =
						inv * Vector4f(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	Matrix4f GLShadowDepthPass::GetViewProjMatrix(Vector3f lightDir, float nearPlane, float farPlane)
	{
		Matrix4f cam_proj = Matrix4f::PerspectiveProjection(renderer->getCamera()->getFov(), ((float)window->getWidth()) / ((float)window->getHeight()), nearPlane, farPlane);
		std::vector<Vector4f> corners = GetFrustumCornersWorldSpace(cam_proj, shadowView);

		//view matrix
		Vector3f centre;
		for (const auto& v : corners)
		{
			centre += v.xyz();
		}
		centre /= float(corners.size());

		Vector3f up = std::abs(lightDir.y) > 0.9999f ? Vector3f(0, 0, 1) : Vector3f(0, 1, 0);
		Vector3f right = lightDir.cross(up).normalise();
		up = right.cross(lightDir).normalise();
		Matrix4f lightView = Matrix4f::View(lightDir, up);

		const Vector4f lightCenter = lightView * Vector4f(centre.x, centre.y, centre.z, 1.0f);
		float radius = 0.0f;
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& corner : corners)
		{
			radius = std::max(radius, (corner.xyz() - centre).length());
			const float z = (lightView * corner).z;
			minZ = std::min(minZ, z);
			maxZ = std::max(maxZ, z);
		}

		// Stabilize the extent as the camera rotates and snap its center to texels.
		radius = std::ceil(radius * 16.0f) / 16.0f;
		// Reserve room for the soft-shadow kernel, plus the snapped center.
		constexpr float filterPadding = 24.0f;
		radius *= SIZE / (SIZE - 2.0f * filterPadding);
		const float texel = 2.0f * radius / SIZE;
		const float x = std::round(lightCenter.x / texel) * texel;
		const float y = std::round(lightCenter.y / texel) * texel;

		// Include casters on the sunward side of the camera slice.
		const float casterReach = std::max(150.0f, farPlane - nearPlane);
		Matrix4f lightProj = Matrix4f::OrthographicProjection(
			x - radius, x + radius, y - radius, y + radius, -(maxZ + casterReach), -minZ);

		return lightProj * lightView;
	}

	std::vector<Matrix4f> GLShadowDepthPass::GetLightSpaceMatrices(Vector3f lightDir)
	{
		std::vector<Matrix4f> ret;

		const float cameraNear = __EngineConfig.rendererNearPlane;
		for (size_t i = 0; i <= shadowCascadeLevels.size(); ++i)
		{
			const float sliceNear = i == 0 ? cameraNear : shadowCascadeLevels[i - 1];
			const float sliceFar = i < shadowCascadeLevels.size() ? shadowCascadeLevels[i] : __EngineConfig.rendererFarPlane;
			const float previousNear = i > 1 ? shadowCascadeLevels[i - 2] : cameraNear;
			const float overlap = (sliceNear - previousNear) * CascadeBlendFraction;
			ret.push_back(GetViewProjMatrix(lightDir, sliceNear - overlap, sliceFar));
		}

		return ret;
	}
};
