#include "Includes.hpp"
#include "GLVoxelRendererPass.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "platform/opengl/texture/GLTexture.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"

namespace Prehistoric
{
	GLVoxelRendererPass::GLVoxelRendererPass(Renderer* renderer)
		: RenderStage(renderer), fbo{ nullptr }, voxel_fbo{ nullptr }
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		voxelImage = man->storeTexture(GLTexture::StorageTrue3D(voxel_size, voxel_size, voxel_size, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, true));
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge));
		man->addReference<Texture>(voxelImage.handle);

		voxel_fbo = std::make_unique<GLFramebuffer>(window);
		voxel_fbo->Bind();
		voxel_fbo->addDepthAttachment(voxel_size, voxel_size, true);
		voxel_fbo->Check();
		voxel_fbo->Unbind();

		fbo = std::make_unique<GLFramebuffer>(window);
		fbo->Bind();
		fbo->addDepthAttachment(width, height, true);
		fbo->addColourAttachment2D(outputImage.pointer);
		fbo->Check();
		fbo->Unbind();

		voxelisationShader = man->loadShader(ShaderName::Voxelisation).value();
		coneTracingShader = man->loadShader(ShaderName::ConeTracing).value();
		man->addReference<Shader>(voxelisationShader.handle);
		man->addReference<Shader>(coneTracingShader.handle);
	}

	GLVoxelRendererPass::~GLVoxelRendererPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Shader>(coneTracingShader.handle);
		man->removeReference<Shader>(voxelisationShader.handle);
		man->removeReference<Texture>(outputImage.handle);
		man->removeReference<Texture>(voxelImage.handle);
	}

	void GLVoxelRendererPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(voxelImage.handle);
		man->removeReference<Texture>(outputImage.handle);
		voxelImage = man->storeTexture(GLTexture::StorageTrue3D(512, 512, 512, R8G8B8A8_LINEAR, Trilinear, ClampToEdge, false));
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge));
		man->addReference<Texture>(voxelImage.handle);

		voxel_fbo->Bind();
		voxel_fbo->addDepthAttachment(voxel_size, voxel_size, true);
		voxel_fbo->Check();
		voxel_fbo->Unbind();

		fbo->Bind();
		fbo->addDepthAttachment(width, height, true);
		fbo->addColourAttachment2D(outputImage.pointer);
		fbo->Check();
		fbo->Unbind();
	}

	void GLVoxelRendererPass::Render()
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();
		voxel_fbo->Bind();
		window->getSwapchain()->SetWindowSize(voxel_size, voxel_size);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
		int previousBoundTextureID;
		glGetIntegerv(GL_TEXTURE_BINDING_3D, &previousBoundTextureID);
		glBindTexture(GL_TEXTURE_3D, ((GLTexture*)voxelImage.pointer)->getTextureID());
		glClearTexImage(((GLTexture*)voxelImage.pointer)->getTextureID(), 0, GL_RGBA, GL_FLOAT, &clearColor);
		glBindTexture(GL_TEXTURE_3D, previousBoundTextureID);
		voxelImage->BindImage();

		voxelisationShader->Bind(nullptr);
		voxelisationShader->UpdateGlobalUniforms(rend);
		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;
			VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
			vbo->Bind(nullptr);

			for (auto material : pipeline.second)
			{
				if (material.first != nullptr)
				{
					voxelisationShader->UpdateMaterialUniforms(material.first);
				}

				for (auto renderer_ : material.second)
				{
					voxelisationShader->UpdateObjectUniforms(renderer_->getParent());

					for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
					{
						vbo->Draw(nullptr, i);
					}
				}
			}
			static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
		}

		voxelImage->Bind();
		voxelImage->GenerateMipmaps();
		voxelImage->Unbind();

		window->getSwapchain()->SetWindowSize(width, height);

		glEnable(GL_DEPTH_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		fbo->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
		fbo->SetDrawAttachments(1, arr);
		fbo->Clear(0.0f);

		coneTracingShader->Bind(nullptr);
		coneTracingShader->UpdateGlobalUniforms(rend);
		for (auto pipeline : rend->getModels3D())
		{
			Pipeline* pl = pipeline.first;
			VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
			vbo->Bind(nullptr);

			for (auto material : pipeline.second)
			{
				if (material.first != nullptr)
				{
					coneTracingShader->UpdateMaterialUniforms(material.first);
				}

				for (auto renderer_ : material.second)
				{
					coneTracingShader->UpdateObjectUniforms(renderer_->getParent());

					for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
					{
						vbo->Draw(nullptr, i);
					}
				}
			}
			static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
		}
		fbo->Unbind();
	}
}
