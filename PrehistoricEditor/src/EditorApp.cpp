#include "EditorApp.h"

#include "platform/opengl/rendering/GLRenderer.h"
#include "prehistoric/application/Application.h"

EditorApp::EditorApp() : Application(true) {
    using namespace Prehistoric;

    auto *rendering = engineLayer->getRenderingEngine();
    auto scene = std::make_unique<Scene>("res/world/sponza_midnight_gi.json", rendering->getWindow(),
                                        rendering->getCamera(), engineLayer->getAssetManager());
    engineLayer->SetScene(scene.get());

    if (__FrameworkConfig.api == OpenGL) {
        auto *voxelPass = static_cast<GLRenderer *>(rendering->getRenderer())->getVoxelPass();
        auto &settings = voxelPass->GetSettings();
        settings.globalIllumination = voxelPass->IsSupported();
        settings.resolutionIndex = 2; // 256³
        settings.traceDepth = 4;
        settings.directionalMips = true;
    }

    editor = new EditorLayer(std::move(scene));
    PushLayer(editor);
}

EditorApp::~EditorApp() {
    layerStack.PopLayer(editor);
    delete editor;
}
