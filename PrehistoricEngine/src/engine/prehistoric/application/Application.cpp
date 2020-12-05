#include "Includes.hpp"
#include "Application.h"

#include "prehistoric/core/events/ApplicationEvent.h"

#include <iostream>

namespace Prehistoric
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent event(1920, 1080);
		//PR_LOG_MESSAGE(event.toString());
		PR_LOG_MESSAGE("Hello, there %s, you age is %d, and 2.5 is %f\n", "Andrew", 10, 2.5f);

		//engine.Start();
	}
};
