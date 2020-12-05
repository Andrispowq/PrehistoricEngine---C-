#include "engine/prehistoric/core/util/Includes.hpp"
#include "Application.h"

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
		engine.Start();
	}
};
