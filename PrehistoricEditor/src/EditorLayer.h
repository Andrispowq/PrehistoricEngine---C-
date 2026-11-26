#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include "prehistoric/core/layers/Layer.h"
#include "prehistoric/core/events/Event.h"
#include "Prehistoric/core/scene/Scene.h"

#include "panels/SceneHierarchyPanel.h"
#include <cstdint>

namespace Prehistoric {
    class Renderer;
}

class EditorLayer : public Prehistoric::Layer {
public:
    EditorLayer(std::unique_ptr<Prehistoric::Scene> scene);

    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;

    virtual void OnDetach() override;

    virtual void Update(float delta) override;

    virtual void ImGUIRender() override;

    virtual void OnEvent(Prehistoric::Event &e) override;

protected:
    void NewButton();

    void OpenButton();

    void SaveButton();

    void QuitButton();

private:
    void DrawRenderSettings(Prehistoric::Renderer &renderer);

    void DrawAntialiasingSettings(Prehistoric::Renderer &renderer);

    void DrawCameraSettings(Prehistoric::Renderer &renderer);

    void DrawLightingSettings(Prehistoric::Renderer &renderer);

    void DrawVoxelSettings(Prehistoric::Renderer &renderer);

    void DrawEnvironmentSettings(Prehistoric::Renderer &renderer);

    void DrawDiagnostics(Prehistoric::Renderer &renderer);

    void DrawEditorInteraction();

    void DrawEnvironmentSelector(Prehistoric::Renderer &renderer);

    std::uint32_t statsDockId = 0;
    std::unique_ptr<SceneHierarchyPanel> scenePanel;
    std::unique_ptr<Prehistoric::Scene> scene;

    float pickDistance = 1000.0f;
    Prehistoric::Vector2f viewportStart;
    Prehistoric::Vector2f viewportSize;

    Prehistoric::GameObject *root;
};

#endif