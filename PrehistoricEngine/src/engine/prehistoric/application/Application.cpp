#include "Includes.hpp"
#include "Application.h"

#include "prehistoric/core/events/ApplicationEvent.h"

namespace Prehistoric
{
	Application::Application()
		: engine{}
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		engine.Start();
	}
};
