#include "PrehistoricScene.h"

#include "prehistoric/application/Application.h"

#include "prehistoric/core/modules/water/Water.h"

#include "prehistoric/core/node/component/scripting/ScriptComponent.h"

#include "prehistoric/core/node/component/physics/PhysicsComponent.h"
#include "prehistoric/core/node/component/physics/colliders/SphereCollider.h"
#include "prehistoric/core/node/component/camera/CameraComponent.h"

PrehistoricScene::PrehistoricScene(const std::string& name, Prehistoric::Window* window, Prehistoric::Camera* camera,
	Prehistoric::AssembledAssetManager* manager)
	: Scene(name, window, camera, manager)
{
	using namespace Prehistoric;

	if (__FrameworkConfig.api == Vulkan)
	{
		AssetManager* man = manager->getAssetManager();

		VertexBufferHandle quad = man->loadVertexBuffer(std::nullopt, "res/models/quad.obj").value();
		VertexBufferHandle sphere = man->loadVertexBuffer(std::nullopt, "res/models/sphere.obj").value();
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

		/*Water* water = new Water(window, camera, manager, Vector3f(0, 200, 0));
		water->UpdateQuadtree();
		sceneRoot->AddChild("water", water);*/

		/*GameObject* slider = new GUISlider(window, manager, 0.0f, 2.0f, Vector3f(0.5f), &__EngineConfig.rendererExposure, sizeof(float), true);
		slider->SetPosition({ 0.5f, 0.5f, 0 });
		slider->SetScale({ 0.125f, 0.05f, 1 });
		sceneRoot->AddChild("slider", slider);

		GameObject* slider2 = new GUISlider(window, manager, 1.0f, 3.4f, Vector3f(0.4f), &__EngineConfig.rendererGamma, sizeof(float), true);
		slider2->SetPosition({ 0.5f, 0.25f, 0 });
		slider2->SetScale({ 0.125f, 0.05f, 1 });
		sceneRoot->AddChild("slider2", slider2);*/
		
		/*GUIElement* reflection = new GUIElement(window, manager, -1, nullptr, 0, true);
		reflection->setTexture(water->getReflectionTexture());
		reflection->SetPosition({ -0.5f, 0.5f, 0 });
		reflection->SetScale({ 0.25f, 0.25f, 1 });
		sceneRoot->AddChild("reflection", reflection);

		GUIElement* refraction = new GUIElement(window, manager, -1, nullptr, 0, true);
		refraction->setTexture(water->getRefractionTexture());
		refraction->SetPosition({ -0.5f, -0.5f, 0 });
		refraction->SetScale({ 0.25f, 0.25f, 1 });
		sceneRoot->AddChild("refraction", refraction);*/

		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/sphere.obj").value();
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

		MaterialHandle mater_script = manager->storeMaterial(new Material(man));
		mater_script->addVector3f(COLOUR, { 1, 0, 0 });
		mater_script->addVector4f(MROT, { 0.5f, 0.5f, 1, 0 });

		MaterialHandle mater_script2 = manager->storeMaterial(new Material(man));
		mater_script2->addVector3f(COLOUR, { 0, 0, 1 });
		mater_script2->addVector4f(MROT, { 0.5f, 0.5f, 1, 0 });

		/*ScriptEngine::Compile("res/scripts");
		ScriptComponent* script = new ScriptComponent("ExampleComponent");
		ScriptComponent* camera_script = new ScriptComponent("CameraController");
		//ScriptComponent* player_script = new ScriptComponent("PlayerController");*/

		/*CameraComponent* cam = new CameraComponent(false);

		GameObject* script_obj = (GameObject*)sceneRoot->AddChild("Example", new GameObject);
		script_obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, mater_script));
		script_obj->AddComponent(SCRIPT_COMPONENT, script);

		GameObject* camera_object = (GameObject*)sceneRoot->AddChild("Camera", new GameObject);
		camera_object->AddComponent(CAMERA_COMPONENT, cam);
		camera_object->AddComponent(SCRIPT_COMPONENT, camera_script);*/

		/*GameObject* script_obj3 = (GameObject*)sceneRoot->AddChild("Player", new GameObject);
		script_obj3->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, mater_script));
		script_obj3->AddComponent(SCRIPT_COMPONENT, player_script);*/

		PhysicsComponent* phys_comp = new PhysicsComponent(1.0f, false);
		phys_comp->setCollider(new SphereCollider());
		phys_comp->AddForce({ 0, -50, 0 });

		GameObject* phys_obj = (GameObject*)sceneRoot->AddChild("phys_obj", new GameObject);
		phys_obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, mater_script));
		phys_obj->AddComponent(PHYSICS_COMPONENT, phys_comp);
		phys_obj->Move({ 0, 100, 0 });

		PhysicsComponent* phys_comp1 = new PhysicsComponent(1.0f, false);
		phys_comp1->setCollider(new SphereCollider());

		GameObject* phys_obj1 = (GameObject*)sceneRoot->AddChild("phys_obj1", new GameObject);
		phys_obj1->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, mater_script2));
		phys_obj1->AddComponent(PHYSICS_COMPONENT, phys_comp1);
		phys_obj1->Move({ 0, 80, 0 });
	}
}

PrehistoricScene::~PrehistoricScene()
{
}
