#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "EditorLayer.h"

#include "prehistoric/core/scene/Scene.h"

class EditorApp : public Prehistoric::Application
{
public:
	EditorApp();
	~EditorApp();

private:
	EditorLayer* editor;

	friend class EditorLayer;
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new EditorApp();
}

#endif