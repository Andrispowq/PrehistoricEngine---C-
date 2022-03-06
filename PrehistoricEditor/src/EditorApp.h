#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "EditorLayer.h"
#include "SpotifyInterface.h"

#include "prehistoric/core/scene/Scene.h"

class EditorApp : public Prehistoric::Application
{
public:
	EditorApp();
	~EditorApp();
private:
	EditorLayer* editor;
	std::unique_ptr<SpotifyInterface> spotifyIF;
	std::unique_ptr<Prehistoric::Scene> scene;
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new EditorApp();
}

#endif