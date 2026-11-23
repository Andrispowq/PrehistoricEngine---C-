#include "EditorApp.h"
#include "platform/opengl/rendering/GLRenderer.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/application/Application.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h" 

#include "prehistoric/core/node/movement/EditorCamera.h"

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

	Camera* cam = engineLayer->getRenderingEngine()->getCamera();//new EditorCamera(); 
	//engineLayer->getRenderingEngine()->ChangeCamera(cam);

	cam->setPosition(Vector3f(0, 5, -2));
	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	GameObject* sun = new GameObject();
	sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
	sun_move_function(sun, 0.0f);

	Scene* scene = new Scene("res/world/testLevel.wrld", window, cam, manager);
	scene->getSceneRoot()->AddChild("sun", sun);
	engineLayer->SetScene(scene);

	//Load in the environment map
	if (__FrameworkConfig.api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(window, manager,
				static_cast<GLRenderer*>(engineLayer->getRenderingEngine()->getRenderer())->getAtmospherePass());
		}


		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap(scene->FindAtmosphere());
		}

		EnvironmentMapRenderer::instance->enabled = true;
	}

	editor = new EditorLayer(scene);
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
	layerStack.PopLayer(editor);
	delete editor;
}
