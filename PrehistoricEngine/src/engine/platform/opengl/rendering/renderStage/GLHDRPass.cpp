#include "Includes.hpp"
#include "GLHDRPass.h"

#include "platform/opengl/rendering/pipeline/GLComputePipeline.h"
#include "platform/opengl/rendering/GLRenderer.h"
#include "platform/opengl/rendering/shaders/postProcessing/GLHDRShader.h"

namespace Prehistoric
{
	GLHDRPass::GLHDRPass(Renderer* renderer)
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

		hdrShader = man->loadShader(ShaderName::HDR).value();
		hdrPipeline = manager->storePipeline(new GLComputePipeline(window, man, hdrShader));
		manager->addReference<Pipeline>(hdrPipeline.handle);

		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->addTextureBinding(0, outputImage.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
	}

	GLHDRPass::~GLHDRPass()
	{
		AssetManager* man = manager->getAssetManager();
		manager->removeReference<Pipeline>(hdrPipeline.handle);
		man->removeReference<Texture>(outputImage.handle);
	}

	void GLHDRPass::OnResized()
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

		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->removeTextureBinding(0);
		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->setInvocationSize({ workGroupsX, workGroupsY, 1 });
		static_cast<GLComputePipeline*>(hdrPipeline.pointer)->addTextureBinding(0, outputImage.pointer, (AccessMask)ComputeAccessFlags::WRITE_ONLY);
	}

	void GLHDRPass::Render()
	{
		PR_PROFILE("HDR post processing");

		GLRenderer* rend = (GLRenderer*)renderer;

		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		hdrPipeline->BindPipeline(nullptr);
		static_cast<GLHDRShader*>(hdrPipeline->getShader())->UpdateUniforms(rend->getVolumetricPostProcessingPass()->getOutputTexture().pointer, {(float)width, (float)height});
		hdrPipeline->RenderPipeline();
		hdrPipeline->UnbindPipeline();
	}
};
