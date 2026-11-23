#include "PrehistoricApp.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "platform/opengl/rendering/GLRenderer.h"

PrehistoricApp::PrehistoricApp()
    : scene{nullptr} {
    using namespace Prehistoric;

    GameObject *audioRoot = new GameObject();
    engineLayer->getRootObject()->AddChild("audioRoot", audioRoot);

    /*GameObject* startupMusic = new GameObject();
	startupMusic->AddComponent(AUDIO_COMPONENT, new AudioComponent("sample://sounds/_Closer.wav", 75.0f, true, true));
	startupMusic->AddComponent("startup", new AudioComponent("sample://sounds/_Eternal.wav", 0.0f, true, false, true));
	startupMusic->GetComponent<AudioComponent>()->PreUpdate(engineLayer);
	
	audioRoot->AddChild("startupMusic", startupMusic);
	engineLayer->getAudioEngine()->Update(0.0f);*/

    scene = std::make_unique<PrehistoricScene>("res/world/world_1.json", engineLayer->getRenderingEngine()->getWindow(),
                                               engineLayer->getRenderingEngine()->getCamera(),
                                               engineLayer->getAssetManager());

    GameObject *sceneRoot = scene->getSceneRoot();
    Camera *cam = engineLayer->getRenderingEngine()->getCamera();

    if (__FrameworkConfig.api != OpenGL) {
        cam->setPosition(Vector3f(0, 5, -2));
    }

    cam->Update(engineLayer->getRenderingEngine()->getWindow(), 0.0f);

    engineLayer->SetScene(scene.get());

    //AssembledAssetManager -> stores the primitives of rendering (Pipelines, Materials) in one place
    //AssetManager -> store the assembling blocks of the primitives (Textures, VertexBuffers, Shaders) in one place
    //Window -> used in a lot of primitives' creation, so it's worth having it around
    AssembledAssetManager *manager = engineLayer->getAssetManager();
    AssetManager *man = manager->getAssetManager();
    Window *window = engineLayer->getRenderingEngine()->getWindow();
    Camera *camera = engineLayer->getRenderingEngine()->getCamera();

    if (__FrameworkConfig.api == OpenGL) {

        GLRenderer *rend = (GLRenderer *) Application::Get().getEngineLayer()->getRenderingEngine()->getRenderer();

        /*GUIElement* shadow_visualiser0 = new GUIElement(window, manager, Vector3f(-1), nullptr, 0, true);
		shadow_visualiser0->setTexture(rend->getShadowDepthPass()->getDepthTexture().pointer);
		shadow_visualiser0->SetPosition({ -0.7f, 0.7f, 0 });
		shadow_visualiser0->SetScale({ 0.125f, 0.125f, 1 });
		sceneRoot->AddChild("shadow_visualiser0", shadow_visualiser0);*/

        VertexBufferHandle vbo = man->loadVertexBuffer(std::nullopt, "res/models/prehistoric/myscene.pmf").value();
        vbo->setFrontFace(FrontFace::CLOCKWISE);
        ShaderHandle shader = man->loadShader(ShaderName::PBR).value();
        PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

        MaterialHandle material = manager->storeMaterial(new Material(man));
        material->addVector3f(COLOUR, {0.64f, 0.53f, 0.23f});
        material->addVector4f(MROT, {1.0f, 0.3f, 1.0f, 0.0f});

        MaterialHandle material2 = manager->storeMaterial(new Material(man));
        material->addVector3f(COLOUR, {0.64f, 0.64f, 0.64f});
        material->addVector4f(MROT, {1.0f, 0.3f, 1.0f, 0.0f});

        GameObject *obj = new GameObject;
        obj->SetPosition({0, -50, 0});
        obj->SetScale({1.f, 1.f, 1.f});
        obj->AddComponent(RENDERER_COMPONENT, new RendererComponent(window, manager, pipeline, material));
        sceneRoot->AddChild("someobj", obj);
    }
}

PrehistoricApp::~PrehistoricApp() {
}