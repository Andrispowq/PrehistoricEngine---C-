#include "Includes.hpp"
#include "NormalMapRenderer.h"

namespace Prehistoric
{
	NormalMapRenderer::NormalMapRenderer(Window* window, AssembledAssetManager* manager, float strength, uint32_t N)
		: manager(manager)
	{
		this->window = window;

		this->strength = strength;
		this->N = N;

		AssetManager* man = manager->getAssetManager();

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = manager->storePipeline(new GLComputePipeline(window, man, man->loadShader(ShaderName::GPGPUNormal).value()));
			manager->addReference<Pipeline>(pipeline.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = manager->storePipeline(new VKComputePipeline(window, man, man->loadShader("gpgpu_normal").value()));
			//manager->addReference<Pipeline>(pipeline.handle);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			normalmap = man->storeTexture(GLTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			man->addReference<Texture>(normalmap.handle);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//normalmap = man->storeTexture(VKTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
			//man->addReference<Pipeline>(normalmap.handle);
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
		manager->getAssetManager()->removeReference<Texture>(normalmap.handle);
		manager->removeReference<Pipeline>(pipeline.handle);
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
