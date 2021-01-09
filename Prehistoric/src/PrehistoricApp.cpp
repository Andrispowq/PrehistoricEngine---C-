#include "PrehistoricApp.h"

#include "prehistoric/core/events/ApplicationEvent.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/common/buffer/VertexBuffer.h"

#define PREHISTORIC_INCLUDE

PrehistoricApp::PrehistoricApp()
	: scene{nullptr}
{
	scene = std::make_unique<PrehistoricScene>(engine.getRootObject(), engine.getRenderingEngine()->getWindow(),
		engine.getRenderingEngine()->getCamera(), engine.getAssetManager(), "res/world/testLevel.wrld");

	engine.setScene(scene.get());

	//Example creation of an object, without bringing the prehistoric namespace in
#if defined(PREHISTORIC_INCLUDE)
	using namespace Prehistoric;
#endif

	//AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
	//AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	AssembledAssetManager* manager = engine.getAssetManager();
	AssetManager* man = manager->getAssetManager();
	Window* window = engine.getRenderingEngine()->getWindow();

	VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/sphere.obj").value();
	vbo->setFrontFace(FrontFace::CLOCKWISE);
	ShaderHandle shader = man->loadShader("pbr").value();
	PipelineHandle pipeline;
	if (FrameworkConfig::api == OpenGL)
		pipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, shader, vbo));
	else
		pipeline = manager->storePipeline(new VKGraphicsPipeline(window, man, shader, vbo));

	MaterialHandle material = manager->storeMaterial(new Material(man));
	material->addVector3f(COLOUR, { 0.64f, 0.53f, 0.23f });
	material->addVector4f(MROT, { 0.3f, 1.0f, 1.0f, 0.0f });

	GameObject* obj = new GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
	engine.AddGameObject("someobj", obj);
}

PrehistoricApp::~PrehistoricApp()
{
}
