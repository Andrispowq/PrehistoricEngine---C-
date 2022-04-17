#include "PrehistoricScene.h"

#include "prehistoric/application/Application.h"

//We go around in a circle, from -range to range on the y and z axes
static void sun_move_function(Prehistoric::GameObject* object, float frameTime)
{
	constexpr float range = 32000.0f;
	constexpr float anglesPerSecond = 0.5f;

	static float angle = 135.0f;

	float x = cos(ToRadians(angle)) * range;
	float y = sin(ToRadians(angle)) * range;
	angle -= (anglesPerSecond * frameTime);

	object->SetPosition({ x, y, 0 });
}

PrehistoricScene::PrehistoricScene(const std::string& name, Prehistoric::Window* window, Prehistoric::Camera* camera,
	Prehistoric::AssembledAssetManager* manager)
	: Scene(name, window, camera, manager)
{
	using namespace Prehistoric;

	if (__FrameworkConfig.api == Vulkan)
	{
		AssetManager* man = manager->getAssetManager();

		VertexBufferHandle quad = man->loadVertexBuffer(std::nullopt, "quadModel").value();
		VertexBufferHandle sphere = man->loadVertexBuffer(std::nullopt, "sphereModel").value();
		ShaderHandle shader = man->loadShader(ShaderName::PBR).value();

		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, quad);
		PipelineHandle pipeline2 = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, sphere);

		MaterialHandle material = manager->storeMaterial(new Material(man));
		material->addTexture(ALBEDO_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_DIF.png", Anisotropic).value());
		material->addTexture(NORMAL_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_NRM.png").value());
		material->addTexture(MROT_MAP, man->loadTexture("res/textures/oakFloor/oakFloor_MROT.png").value());
		material->addVector4f(MROT, { -1, -1, -1, 0 });

		RendererComponent* renderer = new RendererComponent(window, manager, pipeline, material);

		GameObject* obj = new GameObject();
		obj->AddComponent(RENDERER_COMPONENT, renderer);
		obj->Rotate({ -90, 0, 0 });
		obj->Move({ 0, 0, 15 });
		sceneRoot->AddChild("OBJ", obj);

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
				obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline2, material2));
				sceneRoot->AddChild(std::string("obj" + std::to_string(x) + std::to_string(y)), obj);
			}
		}
	}
	else
	{
		AssetManager* man = manager->getAssetManager();

		GameObject* sun = new GameObject();
		//sun->setUpdateFunction(sun_move_function);
		sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
		sun_move_function(sun, 0.0f);
		sceneRoot->AddChild("sun", sun);

		Atmosphere* atm = new Atmosphere(window, manager);
		atm->setSun(sun->GetComponent<Light>());
		sceneRoot->AddChild("atmosphere", atm);
 
		/*Terrain* terrain1 = new Terrain(window, camera, manager, "res/config/terrain_1.json");
		terrain1->UpdateQuadtree();
		sceneRoot->AddChild("terrain1", terrain1);*/

		/*Terrain* terrain0 = new Terrain(window, camera, manager, "res/config/terrain_0.json");
		terrain0->UpdateQuadtree();
		sceneRoot->AddChild("terrain0", terrain0);*/

		/*GameObject* slider = new GUISlider(window, manager, 0.0f, 2.0f, Vector3f(0.5f), &__EngineConfig.rendererExposure, sizeof(float), true);
		slider->SetPosition({ 0.5f, 0.5f, 0 });
		slider->SetScale({ 0.125f, 0.05f, 1 });
		sceneRoot->AddChild("slider", slider);

		GameObject* slider2 = new GUISlider(window, manager, 1.0f, 3.4f, Vector3f(0.4f), &__EngineConfig.rendererGamma, sizeof(float), true);
		slider2->SetPosition({ 0.5f, 0.25f, 0 });
		slider2->SetScale({ 0.125f, 0.05f, 1 });
		sceneRoot->AddChild("slider2", slider2);*/

		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "sphereModel").value();
		ShaderHandle shader = man->loadShader(ShaderName::PBR).value();
		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

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
				sceneRoot->AddChild(std::string("obj" + std::to_string(x) + std::to_string(y)), obj);
			}
		}
	}
}

PrehistoricScene::~PrehistoricScene()
{
}
