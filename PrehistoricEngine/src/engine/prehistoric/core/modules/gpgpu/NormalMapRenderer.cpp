#include "Includes.hpp"
#include "NormalMapRenderer.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	NormalMapRenderer::NormalMapRenderer(Window* window, ResourceStorage* resourceStorage, float strength, uint32_t N)
		: resourceStorage(resourceStorage)
	{
		this->window = window;

		this->strength = strength;
		this->N = N;

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = resourceStorage->storePipeline(new GLComputePipeline(window, resourceStorage, resourceStorage->loadShader("gpgpu_normal").value()));
			resourceStorage->addReference<Pipeline>(pipeline.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = resourceStorage->storePipeline(new VKComputePipeline(window, resourceStorage, resourceStorage->loadShader("gpgpu_normal").value()));
			//resourceStorage->addReference<Pipeline>(pipeline.handle);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			normalmap = resourceStorage->storeTexture(GLTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			resourceStorage->addReference<Texture>(normalmap.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//normalmap = resourceStorage->storeTexture(VKTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			//resourceStorage->addReference<Pipeline>(normalmap.handle);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			GLComputePipeline* glPipe = reinterpret_cast<GLComputePipeline*>(pipeline.pointer);
			glPipe->setInvocationSize({ N / 16, N / 16, 1 });
			glPipe->addTextureBinding(0, normalmap.pointer, WRITE_ONLY);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			/*VKComputePipeline* vkPipe = reinterpret_cast<VKComputePipeline*>(pipeline.pointer);
			vkPipe->setInvocationSize({ float(N / 16), float(N / 16), 1.0f });
			vkPipe->addTextureBinding(0, normalmap.pointer, WRITE_ONLY);*/
		}
	}

	NormalMapRenderer::~NormalMapRenderer()
	{
		resourceStorage->removeReference<Texture>(normalmap.handle);
		resourceStorage->removeReference<Pipeline>(pipeline.handle);
	}

	void NormalMapRenderer::Render(Texture* heightmap)
	{
		pipeline->BindPipeline(nullptr); //FOR NOW

		if (FrameworkConfig::api == OpenGL)
		{
			reinterpret_cast<GLNormalMapShader*>(pipeline->getShader())->UpdateUniforms(heightmap, N, strength);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//reinterpret_cast<VKNormalMapShader*>(pipeline->getShader())->UpdateUnifroms(heightmap, N, strength);
		}

		pipeline->RenderPipeline();
		pipeline->UnbindPipeline();

		normalmap->Bind();
		normalmap->SamplerProperties(Bilinear, Repeat);
	}
};
