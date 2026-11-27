#ifndef SCENE_HIERARCHY_PANEL_H
#define SCENE_HIERARCHY_PANEL_H

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    class RendererComponent;
    class Scene;
}

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel() = default;

    SceneHierarchyPanel(Prehistoric::Scene *scene);

    void SetContext(Prehistoric::Scene *scene);

    void InvalidateSelectionContext() {
        revealSelection = false;
        selectionContext = nullptr;
        selectedSubmesh.reset();
    }

    void SelectViewportHit(Prehistoric::GameObject *object, std::optional<uint32_t> submesh);

    Prehistoric::GameObject *getSelectedObject() const { return selectionContext; }
    std::optional<uint32_t> getSelectedSubmesh() const { return selectedSubmesh; }

    void ImGuiRender();

    void DrawCreateMenu();

    void CreateProceduralObject(bool terrain);

private:
    void DrawGameObjectNode(Prehistoric::GameObject *object);

    void DrawSubmesh(Prehistoric::RendererComponent &component, uint32_t index);

    void DrawComponents(Prehistoric::GameObject *object);

private:
    bool revealSelection = false;
    std::optional<uint32_t> selectedSubmesh;
    Prehistoric::Scene *scene;
    std::string actionError;
    Prehistoric::GameObject *root;
    Prehistoric::GameObject *selectionContext = nullptr;
};

#endif