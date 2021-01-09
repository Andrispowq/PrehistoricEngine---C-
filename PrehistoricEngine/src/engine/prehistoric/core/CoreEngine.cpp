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

		frameTime = 1.0 / FrameworkConfig::windowMaxFPS;
	}

	void CoreEngine::LoadEngines()
	{
		//Root object and manager initialisation
		root = std::make_unique<GameObject>();

		//Engines' initialisation
		renderingEngine = std::make_unique<RenderingEngine>();
		renderingEngine->getWindow()->setEventCallback(std::bind(&CoreEngine::OnEvent, this, std::placeholders::_1));
		audioEngine = std::make_unique<AudioEngine>();

		//Input initialisation
		InputInstance.Init(renderingEngine->getWindow());

		//Manager and the renderers in the rendering engine
		manager = std::make_unique<AssembledAssetManager>(renderingEngine->getWindow());
		renderingEngine->Init(manager.get());
	}

	void CoreEngine::Update(float frameTime)
	{
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

	void CoreEngine::Start()
	{
		if (running)
			return;

		running = true;
		Run();
	}

	void CoreEngine::Stop()
	{
		if (!running)
			return;

		running = false;
	}

	void CoreEngine::Run()
	{
		uint32_t frames = 0;
		double frameCounter = 0;

		long long lastTime = Time::getTimeNanoseconds();
		double unprocessedTime = 0;

		while (running)
		{
			bool render = false;

			long long startTime = Time::getTimeNanoseconds();
			long long passedTime = startTime - lastTime;
			lastTime = startTime;

			//This piece of code is put here so that whenever the window is iconified, the only code that runs is what is absolutely necessary
			//The code before this check is necessary so that the elapsed time is not tracked, but updated (lastTime = startTime)*
			if (InputInstance.IsPause())
			{
				//Checks for updates, so that we can restore the window later
				InputInstance.Update();
				renderingEngine->getWindow()->Input();

				//Makes sure that this loop only runs a 1000 times per second at most, avoiding unnecessary CPU load
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);

				continue;
			}

			unprocessedTime += passedTime / NANOSECOND;
			frameCounter += passedTime;

			while (unprocessedTime > frameTime)
			{
				render = true;
				unprocessedTime -= frameTime;

				if (renderingEngine->getWindow()->ShouldClose())
				{
					Stop();
					break;
				}

				Update((float)frameTime);

				if (frameCounter >= NANOSECOND)
				{
					PR_LOG(CYAN, "FPS: %i\n", frames);
					PR_LOG(CYAN, "Delta time: %f ms\n", frameTime * 1000.0);
					frames = 0;
					frameCounter = 0;
				}
			}

			if (render && !InputInstance.IsPauseRendering())
			{
				Render();
				frames++;
			}
			else
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
		}
	}
};
