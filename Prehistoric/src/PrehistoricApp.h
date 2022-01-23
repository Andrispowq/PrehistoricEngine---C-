#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "scene/PrehistoricScene.h"

#include "SpotifyInterface.h"

class PrehistoricApp : public Prehistoric::Application
{
public:
	PrehistoricApp();
	~PrehistoricApp();
private:
	std::unique_ptr<PrehistoricScene> scene;
	std::unique_ptr<SpotifyInterface> spotifyIF;
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new PrehistoricApp();
}

#endif