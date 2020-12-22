#include "PrehistoricApp.h"

#include "prehistoric/core/events/ApplicationEvent.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/common/buffer/VertexBuffer.h"

#define PREHISTORIC_INCLUDE

PrehistoricApp::PrehistoricApp()
{
	//Example creation of an object, without bringing the prehistoric namespace in
#if defined(PREHISTORIC_INCLUDE)
	using namespace Prehistoric;
	
	AssembledAssetManager* manager = engine.getAssetManager();

	size_t vboID = manager->getAssetManager()->getResource<VertexBuffer>("sphere.obj");
	size_t shaderID = manager->getAssetManager()->getResource<Shader>("pbr");
	size_t pipelineID = manager->loadResource<Pipeline>(
		new GLGraphicsPipeline(engine.getRenderingEngine()->getWindow(), manager->getAssetManager(), shaderID, vboID));

	Material* material = new Material(manager->getAssetManager());
	material->addVector3f(COLOUR, { 0.64f, 0.53f, 0.23f });
	material->addFloat(ROUGHNESS, 0.3f);
	material->addFloat(METALLIC, 1.0f);
	material->addFloat(OCCLUSION, 1.0f);

	size_t materialID = manager->loadResource<Material>(material);

	GameObject* obj = new GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(pipelineID, materialID, engine.getRenderingEngine()->getWindow(), manager));
	engine.AddGameObject("someobj", obj);
#else
	Prehistoric::AssembledAssetManager* manager = engine.getAssetManager();

	size_t vboID = manager->getAssetManager()->getResource<Prehistoric::VertexBuffer>("sphere.obj");
	size_t shaderID = manager->getAssetManager()->getResource<Prehistoric::Shader>("pbr");
	size_t pipelineID = manager->loadResource<Prehistoric::Pipeline>(
		new Prehistoric::GLGraphicsPipeline(engine.getRenderingEngine()->getWindow(), manager->getAssetManager(), shaderID, vboID));

	Prehistoric::Material* material = new Prehistoric::Material(manager->getAssetManager());
	material->addVector3f(COLOUR, { 0.64f, 0.53f, 0.23f });
	material->addFloat(ROUGHNESS, 0.3f);
	material->addFloat(METALLIC, 1.0f);
	material->addFloat(OCCLUSION, 1.0f);

	size_t materialID = manager->loadResource<Prehistoric::Material>(material);

	Prehistoric::GameObject* obj = new Prehistoric::GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new Prehistoric::RendererComponent(pipelineID, materialID, engine.getRenderingEngine()->getWindow(), manager));
	engine.AddGameObject("someobj", obj);
#endif
}

PrehistoricApp::~PrehistoricApp()
{
}
