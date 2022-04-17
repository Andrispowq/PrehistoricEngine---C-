#include "EditorApp.h"

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

SpotifyInterface* sIF;

EditorApp::EditorApp()
	: scene{ nullptr }
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
	spotifyIF->SetDevice(devs[0]->GetId(), true);
	//spotifyIF->PlayTrack("Back to you", 0.0f);
	//spotifyIF->PlayTrack("Story of my life", 50.0f);

	/*SpotifyAPI api = spotifyIF->GetAPI();
	std::vector<PlaylistSimple> playlists = api.GetMyPlaylists().GetItems();
	for (auto playlist : playlists)
	{
		if (playlist.GetName() == "Pop Mix")
		{
			int index = 0;

			std::vector<PlaylistTrack> pl_tracks = api.GetPlaylistTracks(playlist.GetOwner()->GetId(), playlist.GetId()).GetItems();
			std::shared_ptr<Track> track = pl_tracks[index].GetTrack();
			std::string artists;
			for (size_t i = 0; i < track->GetArtists().size(); i++)
			{
				artists += track->GetArtists()[i]->GetName();
				if (i != (track->GetArtists().size() - 1)) artists += ", ";
			}

			PR_LOG_MESSAGE("Now playing %s: %s\n", artists.c_str(), track->GetName().c_str());

			spotifyIF->PlayTrackByID("playlist:" + playlist.GetId(), index);
		}
	}*/

	cam->setPosition(Vector3f(0, 5, -2));
	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	GameObject* sun = new GameObject();
	sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
	sun_move_function(sun, 0.0f);

	scene = std::make_unique<Scene>("res/world/testLevel.wrld", window, cam, manager);
	scene->getSceneRoot()->AddChild("sun", sun);
	engineLayer->SetScene(scene.get());

	//Load in the environment map
	if (__FrameworkConfig.api == OpenGL)
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

	editor = new EditorLayer(scene->getSceneRoot());
	PushLayer(editor);
}

EditorApp::~EditorApp()
{
}
