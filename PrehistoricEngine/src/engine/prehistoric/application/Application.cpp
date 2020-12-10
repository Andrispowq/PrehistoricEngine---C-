#include "Includes.hpp"
#include "Application.h"

#include "prehistoric/core/events/ApplicationEvent.h"

#include <iostream>

namespace Prehistoric
{
	Application::Application()
		: engine{}
	{
		engine.getRenderingEngine()->getWindow()->setEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		engine.Start();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		PR_LOG_MESSAGE(e.toString() + "\n");
	}
};
