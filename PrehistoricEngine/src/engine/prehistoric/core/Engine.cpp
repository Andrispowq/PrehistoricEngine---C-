#include "Includes.hpp"
#include "Engine.h"

namespace Prehistoric
{
	Engine::Engine(const std::string& configPath, const std::string& worldFile)
		: root(nullptr), scene(nullptr), renderingEngine(nullptr), audioEngine(nullptr), manager(nullptr)
	{
		running = false;

		Log::Init();

		LoadConfigurationFiles(configPath);
		LoadEngines();
		LoadScene(worldFile);
	}

	Engine::~Engine()
	{
		//The order is VERY important here, hence the deletion by hand
		Input::DeleteInstance();
		delete scene.release();
		delete root.release();

		delete manager.release();
		delete audioEngine.release();
		delete renderingEngine.release();

		Log::Shutdown();
	}

	void Engine::LoadConfigurationFiles(const std::string& path)
	{
		FrameworkConfig::LoadConfig(path + "/framework.cfg");
		EngineConfig::LoadConfig(path + "/engine.cfg");
		AtmosphereConfig::LoadConfig(path + "/atmosphere.cfg");
		EnvironmentMapConfig::LoadConfig(path + "/environment_map.cfg");
		TerrainConfig::LoadConfig(path + "/terrain.cfg");

		frameTime = 1.0 / FrameworkConfig::windowMaxFPS;
	}

	void Engine::LoadEngines()
	{
		//Root object and manager initialisation
		root = std::make_unique<GameObject>();

		//Engines' initialisation
		renderingEngine = std::make_unique<RenderingEngine>();
		audioEngine = std::make_unique<AudioEngine>();

		//Input initialisation
		InputInstance.Init(renderingEngine->getWindow());

		//Manager and the renderers in the rendering engine
		manager = std::make_unique<AssembledAssetManager>(renderingEngine->getWindow());
		renderingEngine->Init(manager.get());
	}

	void Engine::LoadScene(const std::string& worldFile)
	{
		scene = std::make_unique<Scene>(root.get(), renderingEngine->getWindow(), manager.get(), renderingEngine->getCamera(), worldFile);
	}

	void Engine::AddGameObject(const std::string& name, GameObject* gameobject)
	{
		root->AddChild(name, gameobject);
	}

	void Engine::Start()
	{
		if (running)
			return;

		running = true;
		Run();
	}

	void Engine::Stop()
	{
		if (!running)
			return;

		running = false;
	}

	void Engine::Run()
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

			if (render && !InputInstance.IsPause())
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

	void Engine::Update(float frameTime)
	{
		InputInstance.Update();
		renderingEngine->getWindow()->Input();

		root->PreUpdate(this);

		audioEngine->Update(frameTime);
		renderingEngine->Update(frameTime);
	}

	void Engine::Render()
	{
		root->PreRender(renderingEngine->getRenderer());

		renderingEngine->Render();
	}
};
