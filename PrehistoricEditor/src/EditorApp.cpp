#include "EditorApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "prehistoric/core/modules/environmentMapRenderer/EnvironmentMapRenderer.h" 

#include "prehistoric/core/node/movement/EditorCamera.h"

SpotifyInterface* sIF;

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

	spotifyIF = std::make_unique<SpotifyInterface>("res/private/access.json");
	sIF = spotifyIF.get();
	auto devs = spotifyIF->GetDevices();
	spotifyIF->SetDevice(devs[0]->GetId());
	spotifyIF->PlayTrack("Story of my life", 50.0f);

	cam->setPosition(Vector3f(0, 5, -2));
	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	AudioComponent* startupSound;
	GameObject* start = new GameObject;
	//start->AddComponent("startup", startupSound = new AudioComponent("res/sounds/_FlipReset.wav", 54.0f, true, false, true));
	//start->AddComponent(AUDIO_COMPONENT, startupSound = new AudioComponent("res/sounds/_Saviour.wav", 38.0f, true, true));
	//start->AddComponent(AUDIO_COMPONENT, startupSound = new AudioComponent("res/sounds/_Eternal.wav", 0.0f, true, true));
	//startupSound->setSpatial(true);
	//startupSound->PreUpdate(engineLayer);

	engineLayer->getAudioEngine()->Update(0.0f);

	Scene* scene = new Scene("res/world/testLevel.wrld", window, cam, manager);
	engineLayer->SetScene(scene);

	//Load in the environment map
	if (FrameworkConfig::api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engineLayer->getRenderingEngine()->getWindow(), engineLayer->getAssetManager());
		}

		//EnvironmentMapRenderer::instance->atmosphere = (Atmosphere*)root->getChild("atmosphere");

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}

		EnvironmentMapRenderer::instance->enabled = true;
	}

	editor = new EditorLayer();
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
}
