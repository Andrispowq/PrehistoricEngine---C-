#include "EditorApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h"

//We go around in a circle, from -range to range on the y and z axes
static void sun_move_function(Prehistoric::GameObject* object, float frameTime)
{
	constexpr float range = 32000.0f;
	constexpr float anglesPerSecond = 0.5f;

	static float angle = 190.0f;

	float x = cos(ToRadians(angle)) * range;
	float y = sin(ToRadians(angle)) * range;
	angle -= (anglesPerSecond * frameTime);

	object->SetPosition({ x, y, 0 });
}

EditorApp::EditorApp()
{
	using namespace Prehistoric;

	//AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
	//AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	AssembledAssetManager* manager = engineLayer->getAssetManager();
	AssetManager* man = manager->getAssetManager();
	Window* window = engineLayer->getRenderingEngine()->getWindow();
	GameObject* root = engineLayer->getRootObject();
	Camera* cam = engineLayer->getRenderingEngine()->getCamera();

	cam->setPosition(Vector3f(0, 5, -2));
	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	AudioComponent* startupSound;
	GameObject* start = new GameObject;
	start->AddComponent("startup", startupSound = new AudioComponent("res/sounds/_FlipReset.wav", 54.0f, true, false, true));
	startupSound->setSpatial(true);
	startupSound->PreUpdate(engineLayer);

	engineLayer->getAudioEngine()->Update(0.0);

	WorldLoader loader;
	loader.LoadWorld("res/world/testLevel.wrld", root, window, manager);

	GameObject* sun = new GameObject();
	sun->setUpdateFunction(sun_move_function);
	sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), Vector3f(10000000000.0f)));
	sun_move_function(sun, 0.0f);
	root->AddChild("sun", sun);

	Atmosphere* atmosphere = new Atmosphere(window, manager);
	atmosphere->setSun(sun->GetComponent<Light>());
	//root->AddChild("atmosphere", atmosphere);

	//Load in the environment map
	if (FrameworkConfig::api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engineLayer->getRenderingEngine()->getWindow(), engineLayer->getAssetManager());
		}

		//EnvironmentMapRenderer::instance->atmosphere = atmosphere;

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}

		EnvironmentMapRenderer::instance->enabled = true;
	}

	VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "sphereModel").value();
	ShaderHandle shader = man->loadShader(ShaderName::PBR).value();
	PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

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
	root->AddChild("someobj", obj);

	GameObject* obj2 = new GameObject;
	obj2->SetPosition({ -50, 10, 0 });
	obj2->SetScale({ 1, 1, 1 });
	obj2->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));

	obj2->setUpdateFunction([](GameObject* obj, float delta)
	{
		Vector3f moveDir = { 1.0f, 0.0f, 0.0f };
		obj->Move(moveDir * delta);
	});

	root->AddChild("someobj2", obj2);

	float space = 4.0f;
	float count = 7.0f;

	for (float x = -(count / 2.0f); x <= (count / 2.0f); x++)
	{
		for (float y = -(count / 2.0f); y <= (count / 2.0f); y++)
		{
			MaterialHandle material2 = manager->storeMaterial(new Material(man));
			material2->addVector3f(COLOUR, { 1 });
			material2->addVector4f(MROT, { (y + count / 2.0f) / count, x == -(count / 2.0f) ? 0.05f : (x + count / 2.0f) / count, 1.0f, 0.0f });

			GameObject* obj = new GameObject;
			obj->SetPosition({ x * space, y * space, -50 });
			obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material2));
			root->AddChild(std::string("obj" + std::to_string(x) + std::to_string(y)), obj);
		}
	}

	editor = new EditorLayer();
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
}
