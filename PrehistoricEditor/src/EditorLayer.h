#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include "prehistoric/core/layers/Layer.h"
#include "prehistoric/core/events/Event.h"

#include "panels/SceneHierarchyPanel.h"

class EditorLayer : public Prehistoric::Layer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void Update(float delta) override;
	virtual void ImGUIRender() override;

	virtual void OnEvent(Prehistoric::Event& e) override;

private:
	std::unique_ptr<SceneHierarchyPanel> scenePanel;

	Prehistoric::Vector2f viewportStart;
	Prehistoric::Vector2f viewportSize;
};

#endif