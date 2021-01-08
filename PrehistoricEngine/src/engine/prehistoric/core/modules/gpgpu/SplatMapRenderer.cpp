#include "Includes.hpp"
#include "SplatMapRenderer.h"

namespace Prehistoric
{
	SplatMapRenderer::SplatMapRenderer(Window* window, AssembledAssetManager* manager, uint32_t N)
	{
		this->window = window;
		this->manager = manager;

		this->N = N;

		AssetManager* man = manager->getAssetManager();

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = manager->storePipeline(new GLComputePipeline(window, man, man->loadShader("gpgpu_splat").value()));
			manager->addReference<Pipeline>(pipeline.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = manager->storePipeline(new VKComputePipeline(window, man, man->loadShader("gpgpu_splat").value()));
			//manager->addReference<Pipeline>(pipeline.handle);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			splatmap = man->storeTexture(GLTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			man->addReference<Texture>(splatmap.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//splatmap = man->storeTexture(VKTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			//man->addReference<Pipeline>(splatmap.handle);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			GLComputePipeline* glPipe = reinterpret_cast<GLComputePipeline*>(pipeline.pointer);
			glPipe->setInvocationSize({ N / 16, N / 16, 1 });
			glPipe->addTextureBinding(0, splatmap.pointer, WRITE_ONLY);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			/*VKComputePipeline* vkPipe = reinterpret_cast<VKComputePipeline*>(pipeline);
			vkPipe->setInvocationSize({ float(N / 16), float(N / 16), 1.0f });
			vkPipe->addTextureBinding(0, splatmap.pointer, WRITE_ONLY);*/
		}
	}

	SplatMapRenderer::~SplatMapRenderer()
	{
		manager->getAssetManager()->removeReference<Texture>(splatmap.handle);
		manager->removeReference<Pipeline>(pipeline.handle);
	}

	void SplatMapRenderer::Render(Texture* normalmap)
	{
		pipeline->BindPipeline(nullptr);

		if (FrameworkConfig::api == OpenGL)
		{
			reinterpret_cast<GLSplatMapShader*>(pipeline->getShader())->UpdateUniforms(normalmap, N);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//reinterpret_cast<VKSplatMapShader*>(pipeline->getShader())->UpdateUnifroms(normalmap, N);
		}

		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();

		splatmap->Bind();
		splatmap->SamplerProperties(Bilinear, Repeat);
	}
};
