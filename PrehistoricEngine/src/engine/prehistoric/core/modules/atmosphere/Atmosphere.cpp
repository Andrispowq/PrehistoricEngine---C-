#include "Includes.hpp"
#include "Atmosphere.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	Atmosphere::Atmosphere(Window* window, AssembledAssetManager* manager)
		: window(window)
	{
		SetScale(Vector3f(__EngineConfig.rendererFarPlane / 2.f));
		
		AssetManager* man = manager->getAssetManager();
		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/sphere.obj").value();
		vbo->setFrontFace(FrontFace::DOUBLE_SIDED);

		ShaderHandle shader;

		if (__AtmosphereConfig.scatteringEnabled)
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
