#ifndef PREHISTORIC_APP_H
#define PREHISTORIC_APP_H

#include "Prehistoric.h"

#include "prehistoric/core/events/ApplicationEvent.h"

class PrehistoricApp : public Prehistoric::Application
{
public:
	PrehistoricApp()
	{
	}

	~PrehistoricApp()
	{
	}
};

Prehistoric::Application* Prehistoric::CreateApplication()
{
	return new PrehistoricApp();
}

#endif