#include "Includes.hpp"
#include "GLVolumetricPostProcessingPass.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/pipeline/GLPipeline.h"

namespace Prehistoric
{
	GLVolumetricPostProcessingPass::GLVolumetricPostProcessingPass(Renderer* renderer)
		: RenderStage(renderer)
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(outputImage.handle);

		shader = man->loadShader(ShaderName::VolumetricPostProcessing).value();
		renderPipeline = manager->storePipeline(new GLComputePipeline(window, man, shader));
		manager->addReference<Pipeline>(renderPipeline.handle);

		static_cast<GLComputePipeline*>(renderPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(renderPipeline.pointer)->addTextureBinding(0, outputImage.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
	}

	GLVolumetricPostProcessingPass::~GLVolumetricPostProcessingPass()
	{
		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(outputImage.handle);

		manager->removeReference<Pipeline>(renderPipeline.handle);
	}

	void GLVolumetricPostProcessingPass::OnResized()
	{
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		uint32_t workGroupsX = (width + (width % 16)) / 16;
		uint32_t workGroupsY = (height + (height % 16)) / 16;

		uint32_t numberOfTiles = workGroupsX * workGroupsY;

		AssetManager* man = manager->getAssetManager();
		man->removeReference<Texture>(outputImage.handle);
		outputImage = man->storeTexture(GLTexture::Storage2D(width, height, 1, R8G8B8A8_LINEAR, Bilinear, ClampToEdge, false));
		man->addReference<Texture>(outputImage.handle);

		static_cast<GLComputePipeline*>(renderPipeline.pointer)->removeTextureBinding(0);
		static_cast<GLComputePipeline*>(renderPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(renderPipeline.pointer)->addTextureBinding(0, outputImage.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
	}

	void GLVolumetricPostProcessingPass::Render()
	{
		PR_PROFILE("Volume post processing pass");

		GLRenderer* rend = (GLRenderer*)renderer;

		renderPipeline->BindPipeline(nullptr);
		dynamic_cast<GLVolumetricPostProcessingShader*>(shader.pointer)->UpdateUniforms(rend->getMainPass()->getColourImage().pointer,
				rend->getMainPass()->getPositionMetallicImage().pointer,
				rend->getMainPass()->getAlbedoRoughnessImage().pointer,
				rend->getMainPass()->getNormalImage().pointer, camera, rend->getLights());
		renderPipeline->RenderPipeline();
		renderPipeline->UnbindPipeline();
	}
};
