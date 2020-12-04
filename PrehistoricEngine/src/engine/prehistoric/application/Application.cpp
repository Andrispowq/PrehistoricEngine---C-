#include "engine/prehistoric/core/util/Includes.hpp"
#include "Application.h"

Prehistoric::Application::Application()
{
}

Prehistoric::Application::~Application()
{
}

void Prehistoric::Application::Run()
{
	PR_LOG_MESSAGE("Hello there!\n");
	while (true);
}
