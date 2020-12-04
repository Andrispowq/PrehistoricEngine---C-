#include "engine/Prehistoric.h"

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