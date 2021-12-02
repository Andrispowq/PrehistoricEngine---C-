#include "Includes.hpp"
#include "GLMainPass.h"

#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

namespace Prehistoric
{
	GLMainPass::GLMainPass(Renderer* renderer)
		: RenderStage(renderer), multisampleFBO{ nullptr }, colourFBO{ nullptr }
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		colourImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		bloomImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(colourImage.handle);
		man->addReference<Texture>(bloomImage.handle);

		multisampleFBO = std::make_unique<GLFramebuffer>(window);
		multisampleFBO->Bind();
		multisampleFBO->addDepthAttachment(width, height, true);
		multisampleFBO->addColourAttachmentMultisample2D(0);
		multisampleFBO->addColourAttachmentMultisample2D(1);
		multisampleFBO->Check();
		multisampleFBO->Unbind();

		colourFBO = std::make_unique<GLFramebuffer>(window);
		colourFBO->Bind();
		colourFBO->addDepthAttachment(width, height);
		colourFBO->addColourAttachment2D(colourImage.pointer, 0);
		colourFBO->addColourAttachment2D(bloomImage.pointer, 1);
		colourFBO->Check();
		colourFBO->Unbind();
	}

	GLMainPass::~GLMainPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(colourImage.handle);
		man->removeReference<Texture>(bloomImage.handle);
	}

	void GLMainPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(colourImage.handle);
		man->removeReference<Texture>(bloomImage.handle);
		colourImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		bloomImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(colourImage.handle);
		man->addReference<Texture>(bloomImage.handle);

		multisampleFBO->Bind();
		multisampleFBO->addDepthAttachment(width, height, true);
		multisampleFBO->addColourAttachmentMultisample2D(0);
		multisampleFBO->addColourAttachmentMultisample2D(1);
		multisampleFBO->Check();
		multisampleFBO->Unbind();

		colourFBO->Bind();
		colourFBO->addDepthAttachment(width, height);
		colourFBO->addColourAttachment2D(colourImage.pointer, 0);
		colourFBO->addColourAttachment2D(bloomImage.pointer, 1);
		colourFBO->Check();
		colourFBO->Unbind();
	}

	void GLMainPass::Render()
	{
		PR_PROFILE("Main pass");

		GLRenderer* rend = (GLRenderer*)renderer;

		multisampleFBO->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		multisampleFBO->SetDrawAttachments(2, arr);
		multisampleFBO->Clear(0.0f);

		if (FrameworkConfig::api == OpenGL && EnvironmentMapRenderer::instance && EnvironmentMapRenderer::instance->enabled
			&& EnvironmentMapRenderer::instance->atmosphere == nullptr)
		{
			EnvironmentMapRenderer::instance->RenderCube(camera);
		}

		auto& lights = rend->getLights();
		auto lightCullingPass = rend->getLightCullingPass();

		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(nullptr);
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			lightCullingPass->getLightBuffer()->BindBase(nullptr, 0);
			lightCullingPass->getVisibleLightIndicesBuffer()->BindBase(nullptr, 1);

			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateMaterialUniforms(material.first, 0);

				for (auto renderer : material.second)
				{
					renderer->BatchRender();
				}
			}

			lightCullingPass->getVisibleLightIndicesBuffer()->UnbindBase(1);
			lightCullingPass->getLightBuffer()->UnbindBase(0);

			pl->UnbindPipeline();
		}

		//TODO: enable alpha blending
		for (auto pipeline : rend->getModelsTransparency())
		{
			Pipeline* pl = pipeline.first;

			pl->BindPipeline(nullptr);
			pl->getShader()->UpdateGlobalUniforms(camera, lights);

			lightCullingPass->getLightBuffer()->BindBase(nullptr, 0);
			lightCullingPass->getVisibleLightIndicesBuffer()->BindBase(nullptr, 1);

			for (auto material : pipeline.second)
			{
				pl->getShader()->UpdateMaterialUniforms(material.first, 0);

				for (auto renderer : material.second)
				{
					renderer->BatchRender();
				}
			}

			lightCullingPass->getVisibleLightIndicesBuffer()->UnbindBase(1);
			lightCullingPass->getLightBuffer()->UnbindBase(0);

			pl->UnbindPipeline();
		}

		multisampleFBO->Unbind();
		multisampleFBO->Blit(colourFBO.get(), window->getWidth(), window->getHeight(), 0, 0);
		multisampleFBO->Blit(colourFBO.get(), window->getWidth(), window->getHeight(), 1, 1);
	}
};
