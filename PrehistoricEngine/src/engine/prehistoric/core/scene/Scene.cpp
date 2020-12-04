#include "engine/prehistoric/core/util/Includes.hpp"
#include "Scene.h"

//We go around in a circle, from -range to range on the y and z axes
static void sun_move_function(GameObject* object, float frameTime)
{
	constexpr float range = 32000.0f;
	constexpr float anglesPerSecond = 0.5f;

	static float angle = 180.0f;

	float x = cos(ToRadians(angle)) * range;
	float y = sin(ToRadians(angle)) * range;
	angle -= (anglesPerSecond * frameTime);

	object->SetPosition({ x, y, 0 });
}

Scene::Scene(GameObject* root, Window* window, AssembledAssetManager* manager, Camera* camera)
{
	WorldLoader loader;

	if (FrameworkConfig::api == Vulkan)
	{
		size_t vboID = manager->getAssetManager()->getResource<VertexBuffer>("quad.obj");
		manager->getAssetManager()->getResourceByID<VertexBuffer>(vboID)->setFrontFace(FrontFace::COUNTER_CLOCKWISE);
		size_t shaderID = manager->getAssetManager()->getResource<Shader>("pbr");

		size_t pipelineID = manager->loadResource<Pipeline>(new VKGraphicsPipeline(window, manager->getAssetManager(), shaderID, vboID));

		Material* material = new Material(manager->getAssetManager(), window);
		material->addTexture(ALBEDO_MAP, manager->getAssetManager()->getResource<Texture>("res/textures/oakFloor/oakFloor_DIF.png"));
		material->addTexture(NORMAL_MAP, manager->getAssetManager()->getResource<Texture>("res/textures/oakFloor/oakFloor_NRM.png"));
		material->addTexture(METALLIC_MAP, manager->getAssetManager()->getResource<Texture>("res/textures/oakFloor/oakFloor_MET.png"));
		material->addTexture(ROUGHNESS_MAP, manager->getAssetManager()->getResource<Texture>("res/textures/oakFloor/oakFloor_RGH.png"));

		size_t materialID = manager->loadResource<Material>(material);

		//material->addFloat(METALLIC, 0.7f);
		//material->addFloat(ROUGHNESS, 0.3f);
		material->addFloat(OCCLUSION, 1);

		RendererComponent* renderer = new RendererComponent(pipelineID, materialID, window, manager);
		RendererComponent* renderer2 = new RendererComponent(pipelineID, materialID, window, manager);

		GameObject* obj = new GameObject();
		obj->AddComponent(RENDERER_COMPONENT, renderer);
		obj->Rotate({ -90, 0, 0 });
		obj->Move({ 0, 0, 0 });
		root->AddChild("OBJ", obj);

		GameObject* obj2 = new GameObject();
		obj2->AddComponent(RENDERER_COMPONENT, renderer2);
		obj2->Rotate({ -90, 0, 0 });
		obj2->Move({ 0, 0, -4 });
		root->AddChild("OBJ2", obj2);

		GameObject* light2 = new GameObject();
		light2->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0, 0), Vector3f(1000.0f)));
		light2->Move({ -10, 40, 0 });
		root->AddChild("l", light2);
	}
	else
	{
		loader.LoadWorld("res/world/testLevel.wrld", root, window, manager);

		root->AddChild("Atmosphere", new Atmosphere(window, manager));

		Terrain* terrain = new Terrain(window, camera, manager);
		terrain->UpdateQuadtree();

		root->AddChild("Terrain", terrain);

		GameObject* slider = new GUISlider(window, manager, 0.0f, 2.0f, terrain->getMaps()->getHeightmap(), &EngineConfig::rendererExposure, sizeof(float), true);
		slider->SetPosition({ 0.5f, 0.5f, 0 });
		slider->SetScale({ 0.125f, 0.05f, 1 });
		root->AddChild("slider", slider);

		GameObject* slider2 = new GUISlider(window, manager, 1.0f, 3.4f, terrain->getMaps()->getHeightmap(), &EngineConfig::rendererGamma, sizeof(float), true);
		slider2->SetPosition({ 0.5f, 0.25f, 0 });
		slider2->SetScale({ 0.125f, 0.05f, 1 });
		root->AddChild("slider2", slider2);

		EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(window, manager);
		EnvironmentMapRenderer::instance->GenerateEnvironmentMap();

		/*GameObject* img = new GUIElement(window, manager, EnvironmentMapRenderer::instance->getBRDFMap());
		img->SetPosition({ 0.5f, 0.5f, 0 });
		img->SetScale({ 0.125f, 0.125f, 0 });
		root->AddChild("img", img);*/

		GameObject* sun = new GameObject();
		sun->setUpdateFunction(sun_move_function);
		sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), Vector3f(10000000000.0f), true));
		root->AddChild("sun", sun);

		GameObject* light = new GameObject();
		light->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.56f, 0.34f), Vector3f(300000)));
		light->Move({ 100, 200, -300 });
		root->AddChild("light", light);
	}

	GameObject* audio = new GameObject();
	//audio->AddComponent(AUDIO_COMPONENT, new AudioComponent("res/sounds/bounce.wav"));
	root->AddChild("audio", audio);
}
