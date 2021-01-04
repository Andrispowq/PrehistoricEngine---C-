#include "PrehistoricApp.h"

#include "prehistoric/core/events/ApplicationEvent.h"

#include "prehistoric/core/resources/ResourceStorage.h"
#include "prehistoric/common/buffer/VertexBuffer.h"

#define PREHISTORIC_INCLUDE

#include <vulkan/vulkan.h>

PrehistoricApp::PrehistoricApp()
{
	//Example creation of an object, without bringing the prehistoric namespace in
#if defined(PREHISTORIC_INCLUDE)
	using namespace Prehistoric;
#endif
	
	//AssembledAssetManager -> stores resources created using resources in the AssetManager (Pipelines, Materials)
	//AssetManager -> stores raw resources like Textures, Models, Shaders (TODO: sounds)
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	ResourceStorage* storage = engine.getResourceStorage();
	Window* window = engine.getRenderingEngine()->getWindow();

	VertexBufferHandle vbo = storage->loadVertexBuffer(std::nullopt, "sphere.obj").value();
	ShaderHandle shader = storage->loadShader("pbr").value();
	PipelineHandle pipeline = storage->storePipeline(new GLGraphicsPipeline(window, storage, shader, vbo));

	MaterialHandle material = storage->storeMaterial(new Material(storage));
	material->addVector3f(COLOUR, { 0.64f, 0.53f, 0.23f });
	material->addVector4f(MROT, { 0.3f, 1.0f, 1.0f, 0.0f });

	GameObject* obj = new GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(pipeline, material, window, storage));
	engine.AddGameObject("someobj", obj);
}

PrehistoricApp::~PrehistoricApp()
{
}
