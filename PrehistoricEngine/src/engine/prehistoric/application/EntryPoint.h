#pragma once

#include "engine/platform/Platform.h"
#include "Application.h"

#ifdef PR_WINDOWS_64

extern Prehistoric::Application* Prehistoric::CreateApplication();

int main(int argc, char** args)
{
	Prehistoric::Application* application = Prehistoric::CreateApplication();
	application->Run();
	delete application;

	return 0;
}

#endif