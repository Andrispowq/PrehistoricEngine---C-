#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "scene/PrehistoricScene.h"

class PrehistoricApp : public Prehistoric::Application
{
public:
	PrehistoricApp();
	~PrehistoricApp();
private:
	std::unique_ptr<PrehistoricScene> scene;
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new PrehistoricApp();
}

#endif