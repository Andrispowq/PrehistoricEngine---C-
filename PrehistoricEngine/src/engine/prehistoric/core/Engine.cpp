#include "engine/prehistoric/core/util/Includes.hpp"
#include "Engine.h"

namespace Prehistoric
{
	Engine::Engine()
		: root(nullptr), scene(nullptr), renderingEngine(nullptr), audioEngine(nullptr), manager(nullptr)
	{
		frameTime = 0;

		//Config loading
		FrameworkConfig::LoadConfig("res/config/framework.cfg");
		EngineConfig::LoadConfig("res/config/engine.cfg");
		AtmosphereConfig::LoadConfig("res/config/atmosphere.cfg");
		EnvironmentMapConfig::LoadConfig("res/config/environment_map.cfg");

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

	void Engine::LoadScene(CreateSceneFunction function)
	{
		function(root.get(), renderingEngine->getWindow(), manager.get(), renderingEngine->getCamera());
	}

	void Engine::Input()
	{
		InputInstance.Update();

		renderingEngine->Input();
	}

	//For consistency: Engine updates the root node, and then the engines do the work they need to
	//Most components will register themselves into an engine's list of components, and then the engine can update them
	void Engine::Update(float frameTime)
	{
		this->frameTime = frameTime;

		root->PreUpdate(this);

		audioEngine->Update(frameTime);
		renderingEngine->Update(frameTime);
	}

	void Engine::Render()
	{
		renderingEngine->getWindow()->ClearScreen();
		root->PreRender(renderingEngine->getRenderer());

		renderingEngine->Render();
	}
};
