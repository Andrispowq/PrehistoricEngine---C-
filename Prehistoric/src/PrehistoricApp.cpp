#include "PrehistoricApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "platform/opengl/rendering/GLRenderer.h"

PrehistoricApp::PrehistoricApp()
	: scene{nullptr}
{
	using namespace Prehistoric;

	spotifyIF = std::make_unique<SpotifyInterface>("res/private/access.json");
	spotifyIF->Resume();

	//spotifyIF->PlayTrack("Bad Habits", 38.0f);
	//spotifyIF->PlayTrack("Love me like you do", 118.0f);
	//spotifyIF->PlayTrack("Mood (Remix) feat. Justin Bieber, J Balvin & iann dior", 30.0f);
	//spotifyIF->PlayTrack("Back to you", 0.0f);
	//spotifyIF->PlayTrack("Hate Me", 0.0f);
	//spotifyIF->PlayTrack("Flip Reset", 58.0f);
	//spotifyIF->PlayTrack("Hangover", 0.0f);
	//spotifyIF->PlayTrack("Rajosan", 4.0f);
	//spotifyIF->PlayTrackByID("playlist:37i9dQZF1EQncLwOalG3K7", 0);432

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
				if(i != (track->GetArtists().size() - 1)) artists += ", ";
			}

			PR_LOG_MESSAGE("Now playing %s: %s\n", artists.c_str(), track->GetName().c_str());

			spotifyIF->PlayTrackByID("playlist:" + playlist.GetId(), index);
		}
	}*/

	GameObject* audioRoot = new GameObject();
	engineLayer->getRootObject()->AddChild("audioRoot", audioRoot);

	GameObject* startupMusic = new GameObject();
	//startupMusic->AddComponent(AUDIO_COMPONENT, new AudioComponent("res/sounds/_Closer.wav", 75.0f, true, true));
	//startupMusic->AddComponent("startup", new AudioComponent("res/sounds/_Eternal.wav", 0.0f, true, false, true));
	//startupMusic->GetComponent<AudioComponent>()->PreUpdate(engineLayer);
	
	audioRoot->AddChild("startupMusic", startupMusic);
	engineLayer->getAudioEngine()->Update(0.0f);

	scene = std::make_unique<PrehistoricScene>(/*"res/world/testLevel.wrld"*/"res/world/world_1.json", engineLayer->getRenderingEngine()->getWindow(),
		engineLayer->getRenderingEngine()->getCamera(), engineLayer->getAssetManager()); 

	GameObject* sceneRoot = scene->getSceneRoot();
	Camera* cam = engineLayer->getRenderingEngine()->getCamera();

	if (__FrameworkConfig.api == OpenGL)
	{
		/*cam->setPosition(Vector3f(-178, 102, -47));
		cam->RotateY(-80);
		cam->RotateX(30);*/
	}
	else
	{
		cam->setPosition(Vector3f(0, 5, -2));
	}

	cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

	engineLayer->SetScene(scene.get());

	//AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
	//AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
	//Window -> used in a lot of primitives' creation, so it's worth having it around
	AssembledAssetManager* manager = engineLayer->getAssetManager();
	AssetManager* man = manager->getAssetManager();
	Window* window = engineLayer->getRenderingEngine()->getWindow();
	Camera* camera = engineLayer->getRenderingEngine()->getCamera();

	//Load in the environment map
	if (__FrameworkConfig.api == OpenGL)
	{
		{
			PR_PROFILE("Environment map generation - BRDF Look-up Table");
			EnvironmentMapRenderer::instance = new EnvironmentMapRenderer(engineLayer->getRenderingEngine()->getWindow(), engineLayer->getAssetManager());
		}

		EnvironmentMapRenderer::instance->atmosphere = (Atmosphere*)sceneRoot->getChild("atmosphere");

		{
			PR_PROFILE("Environment map generation - Cubemap, Irradiance, Prefilter map");
			EnvironmentMapRenderer::instance->GenerateEnvironmentMap();
		}

		EnvironmentMapRenderer::instance->enabled = true;
	}

	if (__FrameworkConfig.api == OpenGL)
	{
		/*GameObject* slider3 = new GUISlider(window, manager, 0.0f, 4.0f, Vector3f(0.4f), &EnvironmentMapRenderer::instance->lodRenderedMap, sizeof(float), true);
		static_cast<GUISlider*>(slider3)->setProgress(0.0);
		slider3->SetPosition({ 0.5f, -0.5f, 0 });
		slider3->SetScale({ 0.125f, 0.05f, 1 });
		sceneRoot->AddChild("slider3", slider3);*/

		VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/myscene.obj").value();
		vbo->setFrontFace(FrontFace::CLOCKWISE);
		ShaderHandle shader = man->loadShader(ShaderName::PBR).value();
		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

		MaterialHandle material = manager->storeMaterial(new Material(man));
		material->addVector3f(COLOUR, { 0.64f, 0.53f, 0.23f });
		material->addVector4f(MROT, { 1.0f, 0.3f, 1.0f, 0.0f });

		MaterialHandle material2 = manager->storeMaterial(new Material(man));
		material->addVector3f(COLOUR, { 0.64f, 0.64f, 0.64f });
		material->addVector4f(MROT, { 1.0f, 0.3f, 1.0f, 0.0f });

		GameObject* obj = new GameObject;
		obj->SetPosition({ 0, -50, 0 });
		obj->SetScale({ 1.1f, 1.1f, 1.1f });
		obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
		sceneRoot->AddChild("someobj", obj);

		/*GameObject* obj2 = new GameObject;
		obj2->SetPosition({ -50, 10, 0 });
		obj2->SetScale({ 1, 1, 1 });
		obj2->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));

		sceneRoot->AddChild("someobj2", obj2);*/
	}
}

PrehistoricApp::~PrehistoricApp()
{
}
