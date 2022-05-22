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
	}

	CoreEngine::~CoreEngine()
	{
		//The order is VERY important here, hence the deletion by hand
		delete root.release();

		if (EnvironmentMapRenderer::instance)
		{
			delete EnvironmentMapRenderer::instance;
		}

		renderingEngine->PreRelease();
		delete manager.release();
		delete scriptEngine.release();
		delete physicsEngine.release();
		delete audioEngine.release();
		delete renderingEngine.release();

		Input::DeleteInstance();
		Log::Shutdown();
	}

	void CoreEngine::LoadConfigurationFiles(const std::string& path)
	{
		engineSettings.LoadConfig(path);
	}

	void CoreEngine::LoadEngines()
	{
		//Root object and manager initialisation
		root = std::make_unique<GameObject>();

		//Engines' initialisation
		renderingEngine = std::make_unique<RenderingEngine>();
		audioEngine = std::make_unique<AudioEngine>();
		physicsEngine = std::make_unique<PhysicsEngine>();
		scriptEngine = std::make_unique<ScriptEngine>();

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
		physicsEngine->Update(frameTime);
		scriptEngine->Update(frameTime);
	}

	void CoreEngine::Render()
	{
		{
			PR_PROFILE("PreRender(Renderer*)");
			root->PreRender(renderingEngine->getRenderer());
		}

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
		physicsEngine->OnEvent(event);
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
