#include "Includes.hpp"
#include "GLBloomPass.h"

#include "prehistoric/core/util/ModelFabricator.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLGaussianShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomCombineShader.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLBloomDecomposeShader.h"
#include "platform/opengl/rendering/shaders/gui/GLGUIShader.h"

namespace Prehistoric
{
	GLBloomPass::GLBloomPass(Renderer* renderer)
		: RenderStage(renderer), scratchFBO{nullptr}
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		combinedImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(combinedImage.handle);

		for (uint32_t i = 0; i < BLOOM_PASSES; i++)
		{
			uint32_t local_width = width / (uint32_t)pow(2, i + 1);
			uint32_t local_height = height / (uint32_t)pow(2, i + 1);

			temporaryImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
			bloomImages[i] = man->storeTexture(GLTexture::Storage2D(local_width, local_height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));

			man->addReference<Texture>(temporaryImages[i].handle);
			man->addReference<Texture>(bloomImages[i].handle);
		}

		quad = man->storeVertexBuffer(ModelFabricator::CreateQuad(window));
		quad->setFrontFace(FrontFace::DOUBLE_SIDED);

		decomposeShader = man->loadShader(ShaderName::BloomDecompose).value();
		gaussianShader = man->loadShader(ShaderName::Gaussian).value();
		bloomCombineShader = man->loadShader(ShaderName::BloomCombine).value();
		renderShader = man->loadShader(ShaderName::Gui).value();

		decomposePipeline = manager->storePipeline(new GLComputePipeline(window, man, decomposeShader));
		gaussianPipeline = manager->storePipeline(new GLComputePipeline(window, man, gaussianShader));
		bloomCombinePipeline = manager->storePipeline(new GLComputePipeline(window, man, bloomCombineShader));
		renderPipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, renderShader, quad));
		manager->addReference<Pipeline>(decomposePipeline.handle);
		manager->addReference<Pipeline>(gaussianPipeline.handle);
		manager->addReference<Pipeline>(bloomCombinePipeline.handle);

		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->addTextureBinding(0, rend->getMainPass()->getBloomImage().pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);

		scratchFBO = std::make_unique<GLFramebuffer>(window);
	}

	GLBloomPass::~GLBloomPass()
	{
		AssetManager* man = manager->getAssetManager();
		manager->removeReference<Pipeline>(decomposePipeline.handle);
		manager->removeReference<Pipeline>(gaussianPipeline.handle);
		manager->removeReference<Pipeline>(bloomCombinePipeline.handle);
		manager->removeReference<Pipeline>(renderPipeline.handle);

		man->removeReference<Texture>(combinedImage.handle);
		for (uint32_t i = 0; i < BLOOM_PASSES; i++)
		{
			man->removeReference<Texture>(temporaryImages[i].handle);
			man->removeReference<Texture>(bloomImages[i].handle);
		}
	}

	void GLBloomPass::OnResized()
	{
		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(combinedImage.handle);
		combinedImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(combinedImage.handle);

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

		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->removeTextureBinding(0);
		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(decomposePipeline.pointer)->addTextureBinding(0, rend->getMainPass()->getBloomImage().pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
	}

	void GLBloomPass::Render()
	{
		PR_PROFILE("Bloom pass");

		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		//DECOMPOSE STAGE
		Vector2f size = { (float)width, (float)height };

		GLMainPass* mainPass = rend->getMainPass();
		//GLRayTracingPass* rtxPass = rend->getRayTracingPass();

		/* {
			PR_PROFILE("Decompose pass");
			decomposePipeline->BindPipeline(nullptr);
			static_cast<GLBloomDecomposeShader*>(decomposePipeline->getShader())->UpdateUniforms(mainPass->getColourImage().pointer, 1.0f, size);
			decomposePipeline->RenderPipeline();
			decomposePipeline->UnbindPipeline();
		}*/

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
				static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->addTextureBinding(0, temporaryImages[i].pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);

				Texture* source = mainPass->getBloomImage().pointer;
				if (i > 0)
				{
					source = bloomImages[i - 1].pointer;
				}

				gaussianPipeline->BindPipeline(nullptr);
				static_cast<GLGaussianShader*>(gaussianPipeline->getShader())->UpdateUniforms(source, false, targetDim, targetDim);
				gaussianPipeline->RenderPipeline();

				//HORIZONTAL
				static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->removeTextureBinding(0);
				static_cast<GLComputePipeline*>(gaussianPipeline.pointer)->addTextureBinding(0, bloomImages[i].pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);

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
				static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->addTextureBinding(0, temporaryImages[i].pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);

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
			static_cast<GLComputePipeline*>(bloomCombinePipeline.pointer)->addTextureBinding(0, combinedImage.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);

			bloomCombinePipeline->BindPipeline(nullptr);
			static_cast<GLBloomCombineShader*>(bloomCombinePipeline->getShader())->UpdateUniforms(mainPass->getColourImage().pointer, bloomImages[0].pointer, size);
			bloomCombinePipeline->RenderPipeline();
		}

		//Put it in the colour image, as everything will eventually end up there, making passes optional
		scratchFBO->Bind();
		scratchFBO->addColourAttachment2D(rend->getMainPass()->getColourImage().pointer);
		scratchFBO->Clear(0.0f);

		renderPipeline->BindPipeline(nullptr);
		static_cast<GLGUIShader*>(renderPipeline->getShader())->UpdateCustomUniforms(combinedImage.pointer, -1);
		renderPipeline->RenderPipeline();
		renderPipeline->UnbindPipeline();

		scratchFBO->Unbind();
	}
};
