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
		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "atmosphere").value();
		vbo->setFrontFace(FrontFace::DOUBLE_SIDED);

		ShaderHandle shader;

		if (AtmosphereConfig::scatteringEnabled)
		{
			shader = man->loadShader(ShaderName::AtmosphereScattering).value();
		}
		else
		{
			shader = man->loadShader(ShaderName::Atmosphere).value();
		}

		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

		AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, manager->storeMaterial(nullptr)));
	}
};
