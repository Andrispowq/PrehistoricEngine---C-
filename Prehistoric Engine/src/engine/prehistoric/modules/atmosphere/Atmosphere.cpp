#include "engine/prehistoric/core/util/Includes.hpp"
#include "Atmosphere.h"

#include "engine/prehistoric/core/Engine.h"
#include "engine/prehistoric/resources/AssembledAssetManager.h"

Atmosphere::Atmosphere(Window* window, AssembledAssetManager* manager)
	: window(window)
{
	SetScale(Vector3f(EngineConfig::rendererFarPlane / 2.f));

	size_t vboID = manager->getAssetManager()->getResource<VertexBuffer>("dome/sphericalDome.obj");

	size_t shaderID = -1;
	size_t pipelineID = -1;

	if (FrameworkConfig::api == OpenGL)
	{
		if (AtmosphereConfig::scatteringEnabled)
			shaderID = manager->getAssetManager()->getResource<Shader>("atmosphere_scattering");
		else
			shaderID = manager->getAssetManager()->getResource<Shader>("atmosphere");

		pipelineID = manager->loadResource<Pipeline>(new GLGraphicsPipeline(window, manager->getAssetManager(), shaderID, vboID));
	}
	else if (FrameworkConfig::api == Vulkan)
	{
		/*if (AtmosphereConfig::scatteringEnabled)
			shaderID = manager->getAssetManager()->getResource<Shader>("atmosphere_scattering");
		else
			shaderID = manager->getAssetManager()->getResource<Shader>("atmosphere");*/

		pipelineID = manager->loadResource<Pipeline>(new VKGraphicsPipeline(window, manager->getAssetManager(), shaderID, vboID));
	}

	AddComponent(RENDERER_COMPONENT, new RendererComponent(pipelineID, manager->loadResource<Material>(nullptr), window, manager));

	sunPosition = AtmosphereConfig::sunPosition;
}

void Atmosphere::PreUpdate(Engine* engine)
{
	//sun logic can go in here
	if(engine->getRenderingEngine()->getRenderer()->getSun() != nullptr)
		sunPosition = engine->getRenderingEngine()->getRenderer()->getSun()->getParent()->getWorldTransform().getPosition();
}
