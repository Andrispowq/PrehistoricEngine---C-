#include "Includes.hpp"
#include "CoreEngine.h"

namespace Prehistoric
{
	CoreEngine::CoreEngine(const std::string& configPath, const std::string& worldFile)
		: root(nullptr), renderingEngine(nullptr), audioEngine(nullptr), manager(nullptr)
	{
		running = false;

		Log::Init();

		LoadConfigurationFiles(configPath);
		LoadEngines();
	}

	CoreEngine::~CoreEngine()
	{
		//The order is VERY important here, hence the deletion by hand
		Input::DeleteInstance();
		delete root.release();

		if (EnvironmentMapRenderer::instance)
		{
			delete EnvironmentMapRenderer::instance;
		}

		delete audioEngine.release();
		delete renderingEngine.release();
		delete manager.release();

		Log::Shutdown();
	}

	void CoreEngine::LoadConfigurationFiles(const std::string& path)
	{
		FrameworkConfig::LoadConfig(path + "/framework.cfg");
		EngineConfig::LoadConfig(path + "/engine.cfg");
		AtmosphereConfig::LoadConfig(path + "/atmosphere.cfg");
		EnvironmentMapConfig::LoadConfig(path + "/environment_map.cfg");
		TerrainConfig::LoadConfig(path + "/terrain.cfg");
	}

	void CoreEngine::LoadEngines()
	{
		//Root object and manager initialisation
		root = std::make_unique<GameObject>();

		//Engines' initialisation
		renderingEngine = std::make_unique<RenderingEngine>();
		audioEngine = std::make_unique<AudioEngine>();

		//Manager and the renderers in the rendering engine
		manager = std::make_unique<AssembledAssetManager>(renderingEngine->getWindow());
		renderingEngine->Init(manager.get());
	}

	void CoreEngine::Update(float frameTime)
	{
		this->frameTime = frameTime;

		root->PreUpdate(this);

		InputInstance.Update();
		renderingEngine->getWindow()->Input();

		renderingEngine->Update(frameTime);
		audioEngine->Update(frameTime);
	}

	void CoreEngine::Render()
	{
		root->PreRender(renderingEngine->getRenderer());
		renderingEngine->Render();
	}

	void CoreEngine::BeginRendering()
	{
		renderingEngine->getWindow()->ClearScreen();

		renderingEngine->getRenderer()->PrepareRendering();
	}

	void CoreEngine::EndRendering()
	{
		renderingEngine->getRenderer()->EndRendering();

		RenderingEngine::getStats().drawcalls = 0;
		RenderingEngine::getStats().vertexCount = 0;
		RenderingEngine::getStats().indexCount = 0;
	}

	void CoreEngine::OnEvent(Event& event)
	{
		renderingEngine->OnEvent(event);
		audioEngine->OnEvent(event);
	}

	void CoreEngine::SetScene(Scene* scene)
	{
		this->sceneRoot = scene->getSceneRoot();
		this->scene = scene;

		bool found = false;
		for (auto child : root->getChildren())
		{
			if (child.second == (Node*)sceneRoot)
			{
				child.second->setEnabled(true);
				found = true;
			}
		}

		if (!found)
		{
			root->AddChild(scene->getName(), sceneRoot);
		}
	}

	void CoreEngine::AddGameObject(const std::string& name, GameObject* gameobject)
	{
		root->AddChild(name, gameobject);
	}
};
