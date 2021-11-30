#include "Includes.hpp"
#include "GLRenderer.h"

#include "prehistoric/core/node/component/renderer/RenderableComponent.h"
#include "prehistoric/core/modules/gui/GUIElement.h"
#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"

#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"
#include "prehistoric/core/resources/AssetManager.h"

#include "platform/opengl/rendering/shaders/forwardPlus/GLDepthPassShader.h"
#include "platform/opengl/rendering/shaders/forwardPlus/GLLightCullingPassShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLHDRShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLGaussianShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomCombineShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomDecomposeShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"

namespace Prehistoric
{
	extern bool bloomEnabled;

	GLRenderer::GLRenderer(Window* window, Camera* camera, AssembledAssetManager* manager)
		: Renderer(window, camera, manager), depthFBO{ nullptr }, multisampleFBO{ nullptr },
			colourFBO{ nullptr }, lightBuffer{ nullptr }, visibleLightIndicesBuffer{ nullptr }, 
		scratchFBO{ nullptr }
	{
		depthFBO = std::make_unique<GLFramebuffer>(window);
		multisampleFBO = std::make_unique<GLFramebuffer>(window);
		colourFBO = std::make_unique<GLFramebuffer>(window);

		scratchFBO = std::make_unique<GLFramebuffer>(window);

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		AssetManager* man = manager->getAssetManager();

		depthImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
		colourImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		bloomImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		combinedImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));

		man->addReference<Texture>(depthImage.handle);
		man->addReference<Texture>(colourImage.handle);
		man->addReference<Texture>(bloomImage.handle);
		man->addReference<Texture>(combinedImage.handle);
		man->addReference<Texture>(outputImage.handle);

		for (uint32_t i = 0; i < BLOOM_PASSES; i++)
		{
			uint32_t local_width = width / (uint32_t)pow(2, i + 1);
			uint32_t local_height = height / (uint32_t)pow(2, i + 1);

			temporaryImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
			bloomImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));

			man->addReference<Texture>(temporaryImages[i].handle);
			man->addReference<Texture>(bloomImages[i].handle);
		}

		depthFBO->Bind();
		depthFBO->addColourAttachment2D(depthImage.pointer);
		depthFBO->Check();
		depthFBO->Unbind();

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

		quad = man->storeVertexBuffer(ModelFabricator::CreateQuad(window));
		quad->setFrontFace(FrontFace::DOUBLE_SIDED);

		depthShader = man->loadShader(ShaderName::DepthPass).value();
		lightCullingShader = man->loadShader(ShaderName::LightCullingPass).value();
		decomposeShader = man->loadShader(ShaderName::BloomDecompose).value();
		hdrShader = man->loadShader(ShaderName::HDR).value();
		gaussianShader = man->loadShader(ShaderName::Gaussian).value();
		bloomCombineShader = man->loadShader(ShaderName::BloomCombine).value();
		renderShader = man->loadShader(ShaderName::Gui).value();

		man->addReference<Shader>(depthShader.handle);

		lightCullingPipeline = manager->storePipeline(new GLComputePipeline(window, man, lightCullingShader));
		decomposePipeline = manager->storePipeline(new GLComputePipeline(window, man, decomposeShader));
		hdrPipeline = manager->storePipeline(new GLComputePipeline(window, man, hdrShader));
		gaussianPipeline = manager->storePipeline(new GLComputePipeline(window, man, gaussianShader));
		bloomCombinePipeline = manager->storePipeline(new GLComputePipeline(window, man, bloomCombineShader));
		renderPipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, renderShader, quad));

		manager->addReference<Pipeline>(lightCullingPipeline.handle);
		manager->addReference<Pipeline>(decomposePipeline.handle);
		manager->addReference<Pipeline>(hdrPipeline.handle);
		manager->addReference<Pipeline>(gaussianPipeline.handle);
		manager->addReference<Pipeline>(bloomCombinePipeline.handle);
		manager->addReference<Pipeline>(renderPipeline.handle);

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		lightBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, EngineConfig::lightsMaxNumber * sizeof(InternalLight));
		visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
		static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);

		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->addTextureBinding(0, bloomImage.pointer, WRITE_ONLY);

		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->addTextureBinding(0, outputImage.pointer, WRITE_ONLY);
	}

	GLRenderer::~GLRenderer()
	{
		AssetManager* man = manager->getAssetManager();

		man->removeReference<Shader>(depthShader.handle);

		manager->removeReference<Pipeline>(lightCullingPipeline.handle);
		manager->removeReference<Pipeline>(decomposePipeline.handle);
		manager->removeReference<Pipeline>(hdrPipeline.handle);
		manager->removeReference<Pipeline>(gaussianPipeline.handle);
		manager->removeReference<Pipeline>(bloomCombinePipeline.handle);
		manager->removeReference<Pipeline>(renderPipeline.handle);

		man->removeReference<Texture>(depthImage.handle);
		man->removeReference<Texture>(colourImage.handle);
		man->removeReference<Texture>(bloomImage.handle);
		man->removeReference<Texture>(combinedImage.handle);
		man->removeReference<Texture>(outputImage.handle);

		for (uint32_t i = 0; i < 5; i++)
		{
			man->removeReference<Texture>(temporaryImages[i].handle);
			man->removeReference<Texture>(bloomImages[i].handle);
		}
	}

	void GLRenderer::PrepareRendering()
	{
		if (window->isResized())
		{
			PR_PROFILE("Window resize pass");

			uint32_t width = window->getWidth();
			uint32_t height = window->getHeight();

			uint32_t workGroupsX = (width + (width % 16)) / 16;
			uint32_t workGroupsY = (height + (height % 16)) / 16;

			uint32_t numberOfTiles = workGroupsX * workGroupsY;

			visibleLightIndicesBuffer = std::make_unique<GLShaderStorageBuffer>(window, nullptr, numberOfTiles * sizeof(VisibleIndex) * 1024);

			window->getSwapchain()->SetWindowSize(width, height);

			AssetManager* man = manager->getAssetManager();

			//Recreate the FBO and the images
			man->removeReference<Texture>(depthImage.handle);
			man->removeReference<Texture>(colourImage.handle);
			man->removeReference<Texture>(bloomImage.handle);
			man->removeReference<Texture>(combinedImage.handle);
			man->removeReference<Texture>(outputImage.handle);

			depthImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, D32_LINEAR, Bilinear, ClampToEdge, false));
			colourImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
			bloomImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
			combinedImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
			outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));

			man->addReference<Texture>(depthImage.handle);
			man->addReference<Texture>(colourImage.handle);
			man->addReference<Texture>(bloomImage.handle);
			man->addReference<Texture>(combinedImage.handle);
			man->addReference<Texture>(outputImage.handle);

			for (uint32_t i = 0; i < BLOOM_PASSES; i++)
			{
				man->removeReference<Texture>(temporaryImages[i].handle);
				man->removeReference<Texture>(bloomImages[i].handle);

				uint32_t local_width = width / (uint32_t)pow(2, i + 1);
				uint32_t local_height = height / (uint32_t)pow(2, i + 1);

				temporaryImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
				bloomImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));

				man->addReference<Texture>(temporaryImages[i].handle);
				man->addReference<Texture>(bloomImages[i].handle);
			}

			static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->removeSSBOBinding(0);
			static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->removeSSBOBinding(1);
			static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
			static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(0, (ShaderStorageBuffer*)lightBuffer.get(), READ_ONLY);
			static_cast<GLComputePipeline*>(lightCullingPipeline.pointer)->addSSBOBinding(1, (ShaderStorageBuffer*)visibleLightIndicesBuffer.get(), WRITE_ONLY);

			static_cast<GLComputePipeline*>(decomposePipeline.pointer)->removeTextureBinding(0);
			static_cast<GLComputePipeline*>(decomposePipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
			static_cast<GLComputePipeline*>(decomposePipeline.pointer)->addTextureBinding(0, bloomImage.pointer, WRITE_ONLY);

			static_cast<GLComputePipeline*>(hdrPipeline.pointer)->removeTextureBinding(0);
			static_cast<GLComputePipeline*>(hdrPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
			static_cast<GLComputePipeline*>(hdrPipeline.pointer)->addTextureBinding(0, outputImage.pointer, WRITE_ONLY);

			depthFBO->Bind();
			depthFBO->addColourAttachment2D(depthImage.pointer);
			depthFBO->Check();
			depthFBO->Unbind();

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

			//Recreate the pipelines
			std::vector<Pipeline*> pipes = manager->get<Pipeline>();
			for (Pipeline* pipe : pipes)
			{
				pipe->setViewportSize({ (float)width, (float)height });
				pipe->setScissorSize({ width, height });
				pipe->RecreatePipeline();
			}

			window->setResized(false);
		}
	}

	void GLRenderer::EndRendering()
	{
		window->Render(nullptr);

		//Clear the lists
		models_3d.clear();
		models_transparency.clear();
		models_2d.clear();
		lights.clear();
	}

	void GLRenderer::Render()
	{
		UpdateLightBuffer();

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		{
			PR_PROFILE("Depth pass");
			depthFBO->Bind();
			uint32_t arr[] = { GL_COLOR_ATTACHMENT0 };
			depthFBO->SetDrawAttachments(1, arr);
			depthFBO->Clear(0.0f);

			depthShader->Bind(nullptr);
			depthShader->UpdateGlobalUniforms(camera, lights);

			for (auto pipeline : models_3d)
			{
				Pipeline* pl = pipeline.first;
				VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
				vbo->Bind(nullptr);

				for (auto material : pipeline.second)
				{
					for (auto renderer : material.second)
					{
						depthShader->UpdateObjectUniforms(renderer->getParent());

						for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
						{
							vbo->Draw(nullptr, i);
						}
					}
				}

				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;
				VertexBuffer* vbo = static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer();
				vbo->Bind(nullptr);

				for (auto material : pipeline.second)
				{
					for (auto renderer : material.second)
					{
						depthShader->UpdateObjectUniforms(renderer->getParent());

						for (uint32_t i = 0; i < vbo->getSubmeshCount(); i++)
						{
							vbo->Draw(nullptr, i);
						}
					}
				}

				static_cast<GLGraphicsPipeline*>(pl)->getVertexBuffer()->Unbind();
			}

			depthShader->Unbind();
			depthFBO->Unbind();
		}

		{
			PR_PROFILE("Light culling pass");

			lightCullingPipeline->BindPipeline(nullptr);
			static_cast<GLLightCullingPassShader*>(lightCullingPipeline->getShader())->UpdateUniforms(camera, lights, depthImage.pointer);
			lightCullingPipeline->RenderPipeline();
			lightCullingPipeline->UnbindPipeline();			
		}

		multisampleFBO->Bind();
		uint32_t arr[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		multisampleFBO->SetDrawAttachments(2, arr);
		multisampleFBO->Clear(0.0f);

		{
			PR_PROFILE("Cubemap pass");
			if (FrameworkConfig::api == OpenGL && EnvironmentMapRenderer::instance && EnvironmentMapRenderer::instance->enabled
				&& EnvironmentMapRenderer::instance->atmosphere == nullptr)
			{
				EnvironmentMapRenderer::instance->RenderCube(camera);
			}
		}

		{
			PR_PROFILE("Main pass");

			for (auto pipeline : models_3d)
			{
 				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateGlobalUniforms(camera, lights);

				lightBuffer->BindBase(nullptr, 0);
				visibleLightIndicesBuffer->BindBase(nullptr, 1);

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateMaterialUniforms(material.first, 0);

					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				visibleLightIndicesBuffer->UnbindBase(1);
				lightBuffer->UnbindBase(0);

				pl->UnbindPipeline();
			}

			//TODO: enable alpha blending
			for (auto pipeline : models_transparency)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);
				pl->getShader()->UpdateGlobalUniforms(camera, lights);

				lightBuffer->BindBase(nullptr, 0);
				visibleLightIndicesBuffer->BindBase(nullptr, 1);

				for (auto material : pipeline.second)
				{
					pl->getShader()->UpdateMaterialUniforms(material.first, 0);

					for (auto renderer : material.second)
					{
						renderer->BatchRender();
					}
				}

				visibleLightIndicesBuffer->UnbindBase(1);
				lightBuffer->UnbindBase(0);

				pl->UnbindPipeline();
			}
		}

		multisampleFBO->Unbind();
		multisampleFBO->Blit(colourFBO.get(), window->getWidth(), window->getHeight(), 0, 0);
		multisampleFBO->Blit(colourFBO.get(), window->getWidth(), window->getHeight(), 1, 1);

		{
			PR_PROFILE("Bloom pass");

			//DECOMPOSE STAGE
			Vector2f size = { (float)width, (float)height };

			{
				PR_PROFILE("Decompose pass");
				decomposePipeline->BindPipeline(nullptr);
				static_cast<GLBloomDecomposeShader*>(decomposePipeline->getShader())->UpdateUniforms(colourImage.pointer, 1.0f, size);
				decomposePipeline->RenderPipeline();
				decomposePipeline->UnbindPipeline();
			}

			//GAUSSIAN STAGE
			{
				PR_PROFILE("Gaussian pass");
				for (uint32_t i = 0; i < BLOOM_PASSES; i++)
				{
					uint32_t local_width = width / (uint32_t)pow(2, i + 1);
					uint32_t local_height = height / (uint32_t)pow(2, i + 1);

					uint32_t workGroupsX = (local_width + (local_width % 4)) / 4;
					uint32_t workGroupsY = (local_height + (local_height % 4)) / 4;

					Vector2f targetDim = Vector2f((float)local_width, (float)local_height);

					//VERTICAL
					static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->removeTextureBinding(0);
					static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
					static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->addTextureBinding(0, temporaryImages[i].pointer, WRITE_ONLY);

					Texture* source = bloomImage.pointer;
					if (i > 0)
					{
						source = bloomImages[i - 1].pointer;
					}

					gaussianPipeline->BindPipeline(nullptr);
					static_cast<GLGaussianShader*>(gaussianPipeline->getShader())->UpdateUniforms(source, false, targetDim, targetDim);
					gaussianPipeline->RenderPipeline();

					//HORIZONTAL
					static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->removeTextureBinding(0);
					static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->addTextureBinding(0, bloomImages[i].pointer, WRITE_ONLY);

					gaussianPipeline->BindPipeline(nullptr);
					static_cast<GLGaussianShader*>(gaussianPipeline->getShader())->UpdateUniforms(temporaryImages[i].pointer, true, targetDim, targetDim);
					gaussianPipeline->RenderPipeline();

					gaussianPipeline->UnbindPipeline();
				}
			}

			//COMBINE STAGE
			{
				PR_PROFILE("Combine pass");
				for (int i = BLOOM_PASSES - 2; i >= 0; i--)
				{
					uint32_t local_width = width / (uint32_t)pow(2, i + 1);
					uint32_t local_height = height / (uint32_t)pow(2, i + 1);

					uint32_t workGroupsX = (local_width + (local_width % 4)) / 4;
					uint32_t workGroupsY = (local_height + (local_height % 4)) / 4;

					Vector2f targetDim = Vector2f((float)local_width, (float)local_height);

					//COMBINE
					static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->removeTextureBinding(0);
					static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
					static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->addTextureBinding(0, temporaryImages[i].pointer, WRITE_ONLY);

					bloomCombinePipeline->BindPipeline(nullptr);
					static_cast<GLBloomCombineShader*>(bloomCombinePipeline->getShader())->UpdateUniforms(bloomImages[i + 1].pointer, bloomImages[i].pointer, targetDim);
					bloomCombinePipeline->RenderPipeline();

					//COPY
					scratchFBO->Bind();
					scratchFBO->addColourAttachment2D(bloomImages[i].pointer);
					scratchFBO->Clear(0.0f);
					window->getSwapchain()->SetWindowSize(local_width, local_height);

					renderPipeline->BindPipeline(nullptr);
					static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(temporaryImages[i].pointer, -1);
					renderPipeline->RenderPipeline();
					renderPipeline->UnbindPipeline();

					scratchFBO->Unbind();
				}
			}

			window->getSwapchain()->SetWindowSize(width, height);
			
			//COMBINE
			{
				PR_PROFILE("Combine pass");
				static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->removeTextureBinding(0);
				static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->setInvocationSize({ width / 4, height / 4, 1 });
				static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->addTextureBinding(0, combinedImage.pointer, WRITE_ONLY);

				bloomCombinePipeline->BindPipeline(nullptr);
				static_cast<GLBloomCombineShader*>(bloomCombinePipeline->getShader())->UpdateUniforms(colourImage.pointer, bloomImages[0].pointer, size);
				bloomCombinePipeline->RenderPipeline();

				scratchFBO->Unbind();
			}
		}

		{
			PR_PROFILE("HDR post processing");
			hdrPipeline->BindPipeline(nullptr);
			static_cast<GLHDRShader*>(hdrPipeline->getShader())->UpdateUniforms(bloomEnabled ? combinedImage.pointer : colourImage.pointer, { (float)width, (float)height });
			hdrPipeline->RenderPipeline();
			hdrPipeline->UnbindPipeline();
		}

		{
			PR_PROFILE("Render to screen");
			renderPipeline->BindPipeline(nullptr);
			static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(outputImage.pointer, -1);
			renderPipeline->RenderPipeline();
			renderPipeline->UnbindPipeline();
		}

		//TODO: disable alpha blending and depth testing
		{
			PR_PROFILE("GUI pass");
			for (auto pipeline : models_2d)
			{
				Pipeline* pl = pipeline.first;

				pl->BindPipeline(nullptr);

				for (auto renderer : pipeline.second)
				{
					renderer->BatchRender();
				}

				pl->UnbindPipeline();
			}
		}

	}

	void GLRenderer::UpdateLightBuffer()
	{
		lightBuffer->Bind(nullptr);
		lightBuffer->MapBuffer();
		InternalLight* light = (InternalLight*)lightBuffer->getMappedData();
		memset(light, 0, EngineConfig::lightsMaxNumber * sizeof(InternalLight));

		for (size_t i = 0; i < lights.size(); i++)
		{
			Light* l = lights[i];

			light->position = Vector4f(l->getParent()->getWorldTransform().getPosition(), 1);
			light->colour = Vector4f(l->getColour(), 1);
			light->intensity_radius = Vector4f(l->getIntensity(), l->getRadius(), 0, 0);

			light++;
		}
		
		lightBuffer->UnmapBuffer();
		lightBuffer->Unbind();
	}
};
