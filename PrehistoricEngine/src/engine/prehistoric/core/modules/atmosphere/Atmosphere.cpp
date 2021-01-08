#include "Includes.hpp"
#include "Atmosphere.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	Atmosphere::Atmosphere(Window* window, AssembledAssetManager* manager)
		: window(window)
	{
		SetScale(Vector3f(EngineConfig::rendererFarPlane / 2.f));
		
		AssetManager* man = manager->getAssetManager();
		VertexBufferHandle vbo = man->loadVertexBuffer(OBJLoader::LoadMesh("res/models/dome/", "sphericalDome.obj", ""), "sphericalDome.obj").value();

		ShaderHandle shader;
		PipelineHandle pipeline;

		if (FrameworkConfig::api == OpenGL)
		{
			if (AtmosphereConfig::scatteringEnabled)
				shader = man->loadShader("atmosphere_scattering").value();
			else
				shader = man->loadShader("atmosphere").value();

			pipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, shader, vbo));
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			if (AtmosphereConfig::scatteringEnabled)
				shader = man->loadShader("atmosphere_scattering").value();
			else
				shader = man->loadShader("atmosphere").value();

			pipeline = manager->storePipeline(new VKGraphicsPipeline(window, man, shader, vbo));
		}

		AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, manager->storeMaterial(nullptr)));
	}
};
