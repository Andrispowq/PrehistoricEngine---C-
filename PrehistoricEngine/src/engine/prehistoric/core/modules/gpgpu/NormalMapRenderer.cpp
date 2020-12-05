#include "Includes.hpp"
#include "NormalMapRenderer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	NormalMapRenderer::NormalMapRenderer(Window* window, AssembledAssetManager* manager, float strength, uint32_t N)
		: manager(manager)
	{
		this->window = window;

		this->strength = strength;
		this->N = N;

		//TODO: Create the Vulkan equivalent of the GLComputePipeline
		if (FrameworkConfig::api == OpenGL)
		{
			pipeline = new GLComputePipeline(window, manager->getAssetManager(), manager->getAssetManager()->getResource<Shader>("gpgpu_normal"));
			pipelineID = manager->loadResource<Pipeline>(pipeline);
			manager->addReference<Pipeline>(pipelineID);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//pipeline = new VKComputePipeline(window, manager->getAssetManager(), manager->getAssetManager()->getResource<Shader>("gpgpu_normal"));
			//pipelineID = manager->loadResource<Pipeline>(pipeline);
			//manager->addReference<Pipeline>(pipelineID);
		}

		if (FrameworkConfig::api == OpenGL)
		{
			normalmap = GLTexture::Storage2D(N, N, (uint32_t)(log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear);
			textureID = manager->getAssetManager()->addResource<Texture>(normalmap);
			manager->getAssetManager()->addReference<Texture>(textureID);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//textureID = manager->getAssetManager()->addResource<Texture>(VKTexture::Storage2D(N, N, (uint32_t) (log(N) / log(2)), R8G8B8A8_LINEAR, Bilinear));
		}

		if (FrameworkConfig::api == OpenGL)
		{
			GLComputePipeline* glPipe = reinterpret_cast<GLComputePipeline*>(pipeline);
			glPipe->setInvocationSize({ N / 16, N / 16, 1 });
			glPipe->addTextureBinding(0, normalmap, WRITE_ONLY);
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			/*VKComputePipeline* vkPipe = reinterpret_cast<VKComputePipeline*>(pipeline);
			vkPipe->setInvocationSize({ float(N / 16), float(N / 16), 1.0f });
			vkPipe->addTextureBinding(0, normalmap, WRITE_ONLY);*/
		}
	}

	NormalMapRenderer::~NormalMapRenderer()
	{
		manager->getAssetManager()->removeReference<Texture>(textureID);
		manager->removeReference<Pipeline>(pipelineID);
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
