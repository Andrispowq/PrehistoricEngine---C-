#include "Includes.hpp"
#include "Atmosphere.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	Atmosphere::Atmosphere(Window* window, ResourceStorage* resourceStorage)
		: window(window)
	{
		SetScale(Vector3f(EngineConfig::rendererFarPlane / 2.f));

		VertexBufferHandle vbo = resourceStorage->loadVertexBuffer(OBJLoader::LoadMesh("dome/", "sphericalDome.obj", ""), "dome").value();

		ShaderHandle shader;
		PipelineHandle pipeline;

		if (FrameworkConfig::api == OpenGL)
		{
			if (AtmosphereConfig::scatteringEnabled)
				shader = resourceStorage->loadShader("atmosphere_scattering").value();
			else
				shader = resourceStorage->loadShader("atmosphere").value();

			pipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, shader, vbo));
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			if (AtmosphereConfig::scatteringEnabled)
				shader = resourceStorage->loadShader("atmosphere_scattering").value();
			else
				shader = resourceStorage->loadShader("atmosphere").value();

			pipeline = resourceStorage->storePipeline(new VKGraphicsPipeline(window, resourceStorage, shader, vbo));
		}

		AddComponent(RENDERER_COMPONENT, new RendererComponent(pipeline, resourceStorage->storeMaterial(nullptr), window, resourceStorage));
	}
};
