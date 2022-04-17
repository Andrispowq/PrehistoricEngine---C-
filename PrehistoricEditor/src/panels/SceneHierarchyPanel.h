#ifndef SCENE_HIERARCHY_PANEL_H
#define SCENE_HIERARCHY_PANEL_H

#include "prehistoric/core/node/GameObject.h"

class SceneHierarchyPanel
{
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(Prehistoric::GameObject* root);

	void SetContext(Prehistoric::GameObject* root);
	void InvalidateSelectionContext() { selectionContext = nullptr; }

	void ImGuiRender();

private:
	void DrawGameObjectNode(Prehistoric::GameObject* object);
	void DrawComponents(Prehistoric::GameObject* object);

private:
	Prehistoric::GameObject* root;
	Prehistoric::GameObject* selectionContext = nullptr;
};

#endif