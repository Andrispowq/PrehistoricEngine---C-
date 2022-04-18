#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include "prehistoric/core/layers/Layer.h"
#include "prehistoric/core/events/Event.h"
#include "Prehistoric/core/scene/Scene.h"

#include "panels/SceneHierarchyPanel.h"

class EditorLayer : public Prehistoric::Layer
{
public:
	EditorLayer(Prehistoric::Scene* scene);
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void Update(float delta) override;
	virtual void ImGUIRender() override;

	virtual void OnEvent(Prehistoric::Event& e) override;

protected:
	void NewButton();
	void OpenButton();
	void SaveButton();
	void QuitButton();

private:
	std::unique_ptr<SceneHierarchyPanel> scenePanel;
	std::unique_ptr<Prehistoric::Scene> scene;

	Prehistoric::Vector2f viewportStart;
	Prehistoric::Vector2f viewportSize;

	Prehistoric::GameObject* root;
};

#endif