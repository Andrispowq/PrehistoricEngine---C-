#include "Includes.hpp"
#include "Engine.h"

namespace Prehistoric
{
	Engine::Engine()
		: root(nullptr), scene(nullptr), renderingEngine(nullptr), audioEngine(nullptr), manager(nullptr)
	{
		running = false;

		//Config loading
		FrameworkConfig::LoadConfig("res/config/framework.cfg");
		EngineConfig::LoadConfig("res/config/engine.cfg");
		AtmosphereConfig::LoadConfig("res/config/atmosphere.cfg");
		EnvironmentMapConfig::LoadConfig("res/config/environment_map.cfg");

		frameTime = 1.0 / FrameworkConfig::windowMaxFPS;

		//Rootobject init
		root = std::make_unique<GameObject>();

		//Engines' initialization
		renderingEngine = std::make_unique<RenderingEngine>();
		audioEngine = std::make_unique<AudioEngine>();

		InputInstance.Init(renderingEngine->getWindow());

		manager = std::make_unique<AssembledAssetManager>(renderingEngine->getWindow());
		renderingEngine->Init(manager.get());

		//Loading configs that depend on some engine feature like the window
		TerrainConfig::LoadConfig("res/config/terrain.cfg", renderingEngine->getWindow(), manager->getAssetManager());
		scene = std::make_unique<Scene>(root.get(), renderingEngine->getWindow(), manager.get(), renderingEngine->getCamera());
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
	}

	void Engine::LoadScene()
	{
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

				Update(frameTime);

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
