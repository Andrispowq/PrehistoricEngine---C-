#include "Includes.hpp"
#include "GLDepthPass.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

namespace Prehistoric
{
	GLDepthPass::GLDepthPass(Renderer* renderer)
		: RenderStage(renderer), framebuffer{nullptr}
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		depthTexture = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		man->addReference<Shader>(depthShader.handle);

		framebuffer = std::make_unique<GLFramebuffer>(window);
		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	GLDepthPass::~GLDepthPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Shader>(depthShader.handle);
		man->removeReference<Texture>(depthTexture.handle);
	}

	void GLDepthPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(depthTexture.handle);
		depthTexture = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(depthTexture.handle);

		framebuffer->Bind();
		framebuffer->addColourAttachment2D(depthTexture.pointer);
		framebuffer->Check();
		framebuffer->Unbind();
	}

	void GLDepthPass::Render()
	{
		PR_PROFILE("Depth pass");

		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		framebuffer->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		framebuffer->SetDrawAttachments(1, arr);
		framebuffer->Clear(0.0f);

		depthShader->Bind(nullptr);
		depthShader->UpdateGlobalUniforms(camera, rend->getLights());

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

		depthShader->Unbind();
		framebuffer->Unbind();
	}
};
