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
	UniformBufferObject* _matrices = nullptr;
	Texture* _shadowMap = nullptr;
	std::vector<float> cascadeDistances;

	GLShadowDepthPass::GLShadowDepthPass(Renderer* renderer)
		: RenderStage(renderer), framebuffer{ nullptr }
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		//TODO
		float farPlane = __EngineConfig.rendererFarPlane;
		shadowCascadeLevels = std::vector<float>{ farPlane / 250.0f, farPlane / 100.0f, farPlane / 50.0f, farPlane / 25.0f, farPlane / 10.0f, farPlane / 2.0f};
		cascadeDistances = shadowCascadeLevels;

		matrices = new GLUniformBufferObject(window, nullptr, 16 * 16 * sizeof(float));
		_matrices = matrices;

		AssetManager* man = manager->getAssetManager();
		depthTexture = man->storeTexture(GLTexture::Storage2DArray((uint32_t)SIZE, (uint32_t)SIZE, (int)shadowCascadeLevels.size() + 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		depthShader = man->loadShader(ShaderName::ShadowDepthPass).value();
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
		depthTexture = man->storeTexture(GLTexture::Storage2DArray((uint32_t)SIZE, (uint32_t)SIZE, (int)shadowCascadeLevels.size() + 1, D32_LINEAR, Bilinear, ClampToEdge, false));
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

		Light* sun = nullptr;
		for (auto light : rend->getLights())
		{
			if (light->castShadows())
			{
				sun = light;
				break;
			}
		}

		if (sun == nullptr)
		{
			return;
		}

		//update stuff
		Vector3f sunPos = sun->getParent()->getWorldTransform().getPosition();
		std::vector<Matrix4f> lightMatrices = GetLightSpaceMatrices(sunPos.normalise());

		_shadowMap = depthTexture.pointer;

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
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		framebuffer->SetDrawAttachments(1, arr);
		framebuffer->Clear(0.0f);

		window->getSwapchain()->SetWindowSize((uint32_t)SIZE, (uint32_t)SIZE);

		depthShader->Bind(nullptr);
		dynamic_cast<GLShadowDepthPassShader*>(depthShader.pointer)->UpdateGlobalUniforms(matrices);

		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;
			VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
			vbo->Bind(nullptr);

			for (auto material : pipeline.second)
			{
				if (material.first == nullptr)
				{
					if (material.second.size() < 2)
					{
						continue;
					}

					pl->BindPipeline(nullptr);
					pl->getShader()->UpdateGlobalUniforms(rend);
					pl->getShader()->UpdateMaterialUniforms(material.first, 0);

					for (auto renderer_ : material.second)
					{
						renderer_->BatchRender();
					}

					pl->UnbindPipeline();

					depthShader->Bind(nullptr);
					dynamic_cast<GLShadowDepthPassShader*>(depthShader.pointer)->UpdateGlobalUniforms(matrices);
				}
				else
				{
					/*for (auto renderer_ : material.second)
					{
						depthShader->UpdateObjectUniforms(renderer_->getParent());

						for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
						{
							vbo->Draw(nullptr, i);
						}
					}*/
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
		Matrix4f cam_view = renderer->getCamera()->getViewMatrix();
		std::vector<Vector4f> corners = GetFrustumCornersWorldSpace(cam_proj, cam_view);

		//view matrix
		Vector3f centre;
		for (const auto& v : corners)
		{
			centre += v.xyz();
		}
		centre /= float(corners.size());

		Vector3f up = Vector3f(0, 1, 0);
		Vector3f right = lightDir.cross(up).normalise();
		up = right.cross(lightDir).normalise();
		Matrix4f lightView = Matrix4f::View(lightDir, up);

		//projection
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		//Tuneable
		constexpr float zMult = 5.0f; //10.0f
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}

		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		Matrix4f lightProj = Matrix4f::OrthographicProjection(minX, maxX, minY, maxY, minZ, maxZ);
		return lightProj * lightView;
	}

	std::vector<Matrix4f> GLShadowDepthPass::GetLightSpaceMatrices(Vector3f lightDir)
	{
		std::vector<Matrix4f> ret;

		for (size_t i = 0; i < (shadowCascadeLevels.size() + 1); ++i)
		{
			if (i == 0)
			{
				ret.push_back(GetViewProjMatrix(lightDir, __EngineConfig.rendererNearPlane, shadowCascadeLevels[i]));
			}
			else if (i < shadowCascadeLevels.size())
			{
				ret.push_back(GetViewProjMatrix(lightDir, shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
			}
			else
			{
				ret.push_back(GetViewProjMatrix(lightDir, shadowCascadeLevels[i - 1], __EngineConfig.rendererFarPlane));
			}
		}

		return ret;
	}
};
