#ifndef SCENE_HIERARCHY_PANEL_H
#define SCENE_HIERARCHY_PANEL_H

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    class RendererComponent;
}

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel() = default;

    SceneHierarchyPanel(Prehistoric::GameObject *root);

    void SetContext(Prehistoric::GameObject *root);

    void InvalidateSelectionContext() {
        revealSelection = false;
        selectionContext = nullptr;
        selectedSubmesh.reset();
    }

    void SelectViewportHit(Prehistoric::GameObject *object, std::optional<uint32_t> submesh);

    Prehistoric::GameObject *getSelectedObject() const { return selectionContext; }
    std::optional<uint32_t> getSelectedSubmesh() const { return selectedSubmesh; }

    void ImGuiRender();

private:
    void DrawGameObjectNode(Prehistoric::GameObject *object);

    void DrawSubmesh(Prehistoric::RendererComponent &component, uint32_t index);

    void DrawComponents(Prehistoric::GameObject *object);

private:
    bool revealSelection = false;
    std::optional<uint32_t> selectedSubmesh;
    Prehistoric::GameObject *root;
    Prehistoric::GameObject *selectionContext = nullptr;
};

#endif