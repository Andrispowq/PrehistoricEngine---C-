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

	//We go around in a circle, from -range to range on the y and z axes
	void Atmosphere::sunMoveFunction(Prehistoric::GameObject* object, float frameTime)
	{
		constexpr float range = 32000.0f;
		constexpr float anglesPerSecond = 0.5f;

		static float angle = 130.0f;

		float x = cos(ToRadians(angle)) * range;
		float y = sin(ToRadians(angle)) * range;
		angle -= (anglesPerSecond * frameTime);

		object->SetPosition({ x, y, 0 });
	}
};
