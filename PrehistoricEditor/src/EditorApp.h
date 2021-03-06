#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "EditorLayer.h"

class EditorApp : public Prehistoric::Application
{
public:
	EditorApp();
	~EditorApp();
private:
	EditorLayer* editor;
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new EditorApp();
}

#endif