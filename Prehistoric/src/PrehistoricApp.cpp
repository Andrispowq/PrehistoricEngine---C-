#include "PrehistoricApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

PrehistoricApp::PrehistoricApp()
	: scene{nullptr}
{
	using namespace Prehistoric;

	GameObject* audio = new GameObject();
	audio->AddComponent(AUDIO_COMPONENT, new AudioComponent("res/sounds/_SolidGround.wav", 57.0f));
	audio->GetComponent<AudioComponent>()->PreUpdate(engineLayer);
	engineLayer->getRootObject()->AddChild("audio", audio);

	engineLayer->getAudioEngine()->Update(0.0f);
	
	GameObject* sceneRoot = new GameObject();
	scene = std::make_unique<PrehistoricScene>("scene0", sceneRoot, engineLayer->getRenderingEngine()->getWindow(),
		engineLayer->getRenderingEngine()->getCamera(), engineLayer->getAssetManager(), "res/world/testLevel.wrld");

	engineLayer->SetScene(scene.get());

	//Load in the environment map
	if (FrameworkConfig::api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engineLayer->getRenderingEngine()->getWindow(), engineLayer->getAssetManager());
		}

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}
	}

	//AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
	//AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	AssembledAssetManager* manager = engineLayer->getAssetManager();
	AssetManager* man = manager->getAssetManager();
	Window* window = engineLayer->getRenderingEngine()->getWindow();

	GameObject* slider3 = new GUISlider(window, manager, 0.0f, 4.0f, Vector3f(0.4f), &EnvironmentMapRenderer::instance->lodRenderedMap, sizeof(float), true);
	static_cast<GUISlider*>(slider3)->setProgress(0.0);
	slider3->SetPosition({ 0.5f, -0.5f, 0 });
	slider3->SetScale({ 0.125f, 0.05f, 1 });
	sceneRoot->AddChild("slider3", slider3);

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

	MaterialHandle material2 = manager->storeMaterial(new Material(man));
	material->addVector3f(COLOUR, { 0.64f, 0.64f, 0.64f });
	material->addVector4f(MROT, { 1.0f, 0.3f, 1.0f, 0.0f });

	GameObject* obj = new GameObject;
	obj->SetPosition({ 0, 200, 0 });
	obj->SetScale({ 10, 10, 10 });
	obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
	sceneRoot->AddChild("someobj", obj);

	GameObject* obj2 = new GameObject;
	obj2->SetPosition({ -50, 10, 0 });
	obj2->SetScale({ 1, 1, 1 });
	obj2->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));

	obj2->setUpdateFunction([](GameObject* obj, float delta)
	{
		Vector3f moveDir = { 1.0f, 0.0f, 0.0f };
		obj->Move(moveDir * delta);
	});

	sceneRoot->AddChild("someobj2", obj2);
}

PrehistoricApp::~PrehistoricApp()
{
}
