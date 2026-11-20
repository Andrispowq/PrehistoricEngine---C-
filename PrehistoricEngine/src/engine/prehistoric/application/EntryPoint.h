#pragma once

#include "platform/Platform.h"
#include "Application.h"

#if defined(PR_WINDOWS_64) || defined(PR_PLATFORM_MACOS)

extern Prehistoric::Application* Prehistoric::CreateApplication();

int main(int argc, char** args)
{
	Prehistoric::Application* application = Prehistoric::CreateApplication();
	application->Run();
	delete application;

	return 0;
}

#endif