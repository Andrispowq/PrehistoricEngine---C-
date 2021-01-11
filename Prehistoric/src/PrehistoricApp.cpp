#include "PrehistoricApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

PrehistoricApp::PrehistoricApp()
	: scene{nullptr}
{
	using namespace Prehistoric;

	GameObject* sceneRoot = new GameObject();
	scene = std::make_unique<PrehistoricScene>("scene0", sceneRoot, engine.getRenderingEngine()->getWindow(),
		engine.getRenderingEngine()->getCamera(), engine.getAssetManager(), "res/world/testLevel.wrld");

	engine.SetScene(scene.get());

	//Load in the environment map
	if (FrameworkConfig::api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engine.getRenderingEngine()->getWindow(), engine.getAssetManager());
		}

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}
	}

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
	material->addVector4f(MROT, { 1.0f, 0.3f, 1.0f, 0.0f });

	GameObject* obj = new GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
	engine.AddGameObject("someobj", obj);
}

PrehistoricApp::~PrehistoricApp()
{
}
