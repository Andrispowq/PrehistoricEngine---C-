#include "Includes.hpp"
#include "CoreEngine.h"

namespace Prehistoric {
    CoreEngine::CoreEngine(const std::string &configPath, const std::string &worldFile)
        : root(nullptr), renderingEngine(nullptr), audioEngine(nullptr), manager(nullptr) {
        running = false;

        Log::Init();

        LoadConfigurationFiles(configPath);
    }

    CoreEngine::~CoreEngine() {
        //The order is VERY important here, hence the deletion by hand
        root.reset();

        renderingEngine->PreRelease();
        manager.reset();
#ifdef PR_WINDOWS_64
        scriptEngine.reset();
#endif
        physicsEngine.reset();
        audioEngine.reset();
        renderingEngine.reset();

        Input::DeleteInstance();
        Log::Shutdown();
    }

    void CoreEngine::LoadConfigurationFiles(const std::string &path) {
        engineSettings.LoadConfig(path);
    }

    void CoreEngine::LoadEngines() {
        //Root object and manager initialisation
        root = std::make_unique<GameObject>();

        //Engines' initialisation
        renderingEngine = std::make_unique<RenderingEngine>();
        audioEngine = std::make_unique<AudioEngine>();
        physicsEngine = std::make_unique<PhysicsEngine>();
#ifdef PR_WINDOWS_64
        scriptEngine = std::make_unique<ScriptEngine>();
#endif

        //Manager and the renderers in the rendering engine
        manager = std::make_unique<AssembledAssetManager>(renderingEngine->getWindow());
        renderingEngine->Init(manager.get());
    }

    void CoreEngine::Update(float frameTime) {
        this->frameTime = frameTime;

        root->PreUpdate(this);

        InputInstance.Update();
        renderingEngine->getWindow()->Input();

        renderingEngine->Update(frameTime);
        audioEngine->Update(frameTime);
        physicsEngine->Update(frameTime);
#ifdef PR_WINDOWS_64
        scriptEngine->Update(frameTime);
#endif
    }

    void CoreEngine::Render() {
        RenderingEngine::getStats() = {};
        {
            PR_PROFILE("PreRender(Renderer*)");
            root->PreRender(renderingEngine->getRenderer());
        }

        renderingEngine->Render();
    }

    void CoreEngine::BeginRendering() {
        renderingEngine->getWindow()->ClearScreen();
        renderingEngine->getRenderer()->PrepareRendering();
    }

    void CoreEngine::EndRendering() {
        renderingEngine->getRenderer()->EndRendering();
    }

    void CoreEngine::OnEvent(Event &event) {
        renderingEngine->OnEvent(event);
        audioEngine->OnEvent(event);
        physicsEngine->OnEvent(event);
#ifdef PR_WINDOWS_64
        scriptEngine->OnEvent(event);
#endif
    }

    void CoreEngine::SetScene(Scene *scene) {
        scene->ValidateSky();
        if (this->sceneRoot && this->sceneRoot != scene->getSceneRoot()) this->sceneRoot->setEnabled(false);
        this->sceneRoot = scene->getSceneRoot();
        this->scene = scene;
        renderingEngine->getRenderer()->screenSpaceReflections = scene->screenSpaceReflections;
        renderingEngine->getRenderer()->bloomIntensity = scene->bloomIntensity;
        renderingEngine->getRenderer()->ambientOcclusion = scene->ambientOcclusion;
        renderingEngine->getRenderer()->depthOfField = scene->depthOfField;
        renderingEngine->getRenderer()->temporal = scene->temporal;
        renderingEngine->getRenderer()->ResetTemporalHistory();
        renderingEngine->getRenderer()->RequestEnvironmentBake();

        if (const auto &pose = scene->getInitialCamera()) {
            auto *camera = renderingEngine->getCamera();
            camera->setPosition(pose->position);
            camera->setForward(pose->forward);
            camera->setUp(pose->up);
            if (pose->bloom) renderingEngine->getRenderer()->setBloomEnabled(*pose->bloom);
            if (pose->exposure) engineSettings.engineConfig.rendererExposure = *pose->exposure;
            camera->Update(renderingEngine->getWindow(), 0.0f);
        }

        bool found = false;
        for (auto child: root->getChildrenByID()) {
            if (child.second == (Node *) sceneRoot) {
                child.second->setEnabled(true);
                found = true;
            }
        }

        if (!found) {
            root->AddChild(scene->getName(), sceneRoot);
        }
    }

    void CoreEngine::AddGameObject(const std::string &name, GameObject *gameobject) {
        root->AddChild(name, gameobject);
    }
};