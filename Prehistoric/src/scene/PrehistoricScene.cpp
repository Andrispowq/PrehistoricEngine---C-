#include "PrehistoricScene.h"

//We go around in a circle, from -range to range on the y and z axes
static void sun_move_function(Prehistoric::GameObject* object, float frameTime)
{
	constexpr float range = 32000.0f;
	constexpr float anglesPerSecond = 0.0f;//0.5f;

	static float angle = 170.0f;

	float x = cos(ToRadians(angle)) * range;
	float y = sin(ToRadians(angle)) * range;
	angle -= (anglesPerSecond * frameTime);

	object->SetPosition({ x, y, 0 });
}

PrehistoricScene::PrehistoricScene(const std::string& name, Prehistoric::GameObject* root, Prehistoric::Window* window, Prehistoric::Camera* camera,
	Prehistoric::AssembledAssetManager* manager, const std::string& worldFile)
	: Scene(name, root, window, camera, manager)
{
	using namespace Prehistoric;

	WorldLoader loader;

	if (FrameworkConfig::api == Vulkan)
	{
		AssetManager* man = manager->getAssetManager();

		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/quad.obj").value();
		vbo->setFrontFace(FrontFace::COUNTER_CLOCKWISE);
		ShaderHandle shader = man->loadShader("pbr").value();

		PipelineHandle pipeline = manager->storePipeline(new VKGraphicsPipeline(window, man, shader, vbo));

		MaterialHandle material = manager->storeMaterial(new Material(man));
		material->addTexture(ALBEDO_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_DIF.png", Anisotropic).value());
		material->addTexture(NORMAL_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_NRM.png").value());
		material->addTexture(MROT_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_MROT.png").value());

		material->addVector4f(MROT, { 1, 0.05f, -1, 0 });

		RendererComponent* renderer = new RendererComponent(window, manager, pipeline, material);
		//RendererComponent* renderer2 = new RendererComponent(window, manager, pipeline, material);

		GameObject* obj = new GameObject();
		obj->AddComponent(RENDERER_COMPONENT, renderer);
		obj->Rotate({ -90, 0, 0 });
		obj->Move({ 0, 0, 0 });
		root->AddChild("OBJ", obj);

		/*GameObject* obj2 = new GameObject();
		obj2->AddComponent(RENDERER_COMPONENT, renderer2);
		obj2->Rotate({ -90, 0, 0 });
		obj2->Move({ 0, 0, -4 });
		root->AddChild("OBJ2", obj2);*/

		GameObject* light2 = new GameObject();
		light2->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0, 0), Vector3f(1000.0f)));
		light2->Move({ -10, 40, 0 });
		root->AddChild("l", light2);
	}
	else
	{
		AssetManager* man = manager->getAssetManager();

		loader.LoadWorld(worldFile, root, window, manager);

		/*GameObject* sun = new GameObject();
		sun->setUpdateFunction(sun_move_function);
		sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), Vector3f(10000000000.0f)));
		root->AddChild("sun", sun);*/

		/*Atmosphere* atm = new Atmosphere(window, resourceStorage);
		atm->setSun(sun->GetComponent<Light>());
		root->AddChild("Atmosphere", atm);*/

		Terrain* terrain = new Terrain(window, camera, manager, "res/config/terrain_0.cfg");
		terrain->UpdateQuadtree();

		root->AddChild("Terrain", terrain);

		GameObject* slider = new GUISlider(window, manager, 0.0f, 2.0f, terrain->getMaps()->getHeightmap(), &EngineConfig::rendererExposure, sizeof(float), true);
		slider->SetPosition({ 0.5f, 0.5f, 0 });
		slider->SetScale({ 0.125f, 0.05f, 1 });
		root->AddChild("slider", slider);

		GameObject* slider3 = new GUISlider(window, manager, 0.0f, 2.0f, terrain->getMaps()->getHeightmap(), &EngineConfig::rendererExposure, sizeof(float), true);
		slider3->SetPosition({ 0.5f, -0.5f, 0 });
		slider3->SetScale({ 0.125f, 0.125f, 1 });
		root->AddChild("slider3", slider3);

		GameObject* slider2 = new GUISlider(window, manager, 1.0f, 3.4f, terrain->getMaps()->getHeightmap(), &EngineConfig::rendererGamma, sizeof(float), true);
		slider2->SetPosition({ 0.5f, 0.25f, 0 });
		slider2->SetScale({ 0.125f, 0.05f, 1 });
		root->AddChild("slider2", slider2);

		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/sphere.obj").value();
		vbo->setFrontFace(FrontFace::CLOCKWISE);
		ShaderHandle shader = man->loadShader("pbr").value();

		PipelineHandle pipeline = manager->storePipeline(new GLGraphicsPipeline(window, man, shader, vbo));

		float space = 4.0f;
		float count = 7.0f;

		for (float x = -(count / 2.0f); x <= (count / 2.0f); x++)
		{
			for (float y = -(count / 2.0f); y <= (count / 2.0f); y++)
			{
				MaterialHandle material = manager->storeMaterial(new Material(man));
				material->addVector3f(COLOUR, { 1 });
				material->addVector4f(MROT, { (y + count / 2.0f) / (count + 1.0f), x == -(count / 2.0f) ? 0.05f : (x + count / 2.0f) / (count + 1.0f), 1.0f, 0.0f });

				GameObject* obj = new GameObject;
				obj->SetPosition({ x * space, y * space, -50 });
				obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
				root->AddChild(std::string("obj" + std::to_string(x) + std::to_string(y)), obj);
			}
		}
	}

	//GameObject* audio = new GameObject();
	//audio->AddComponent(AUDIO_COMPONENT, new AudioComponent("res/sounds/bounce.wav"));
	//root->AddChild("audio", audio);
}

PrehistoricScene::~PrehistoricScene()
{
}
