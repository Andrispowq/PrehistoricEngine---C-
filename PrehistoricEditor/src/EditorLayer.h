#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include "prehistoric/core/layers/Layer.h"
#include "prehistoric/core/events/Event.h"

class EditorLayer : public Prehistoric::Layer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void Update(float delta) override;
	virtual void Render() override;

	virtual void OnEvent(Prehistoric::Event& e) override;

private:
};

#endif