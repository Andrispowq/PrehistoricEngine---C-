#ifndef LOG_H
#define LOG_H

#include "engine/prehistoric/core/util/Includes.hpp"
#include "ColourCodes.h"
#include "engine/platform/Platform.h"
#include "engine/prehistoric/core/util/time/Timer.h"
#include <time.h>
#include <ctime>
#include <stdio.h>

//We will ignore messages and warnings in release builds, though errors and runtime errors are useful to have there too
#if defined(PR_DEBUG)
	//Use this macro at the end of a function with its name to profile it
	#define PR_PROFILE(x) ::Prehistoric::Timer(x)
	
	#define PR_LOG_MESSAGE(...)        ::Prehistoric::Log::Log(WHITE, __VA_ARGS__)
	#define PR_LOG_WARNING(...)        ::Prehistoric::Log::Log(YELLOW, __VA_ARGS__)
	#define PR_LOG_ERROR(...)          ::Prehistoric::Log::Log(RED, __VA_ARGS__)
	#define PR_LOG_RUNTIME_ERROR(...)  ::Prehistoric::Log::Log_RuntimeError(RED, __VA_ARGS__)
	#define PR_LOG(x, ...)             ::Prehistoric::Log::Log(x, __VA_ARGS__)
#else
	#define PR_PROFILE(x)
	
	#define PR_LOG_MESSAGE(...)
	#define PR_LOG_WARNING(...)
	#define PR_LOG_ERROR(...)          ::Prehistoric::Log::Log(RED, __VA_ARGS__)
	#define PR_LOG_RUNTIME_ERROR(...)  ::Prehistoric::Log::Log_RuntimeError(RED, __VA_ARGS__)
	#define PR_LOG(x, ...)             ::Prehistoric::Log::Log(x, __VA_ARGS__)
#endif

namespace Prehistoric
{
	namespace Log
	{
		template<typename... Args> void Log(const std::string& colour, const std::string& message, Args... args)
		{
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			printf("%s", ("\033" + colour).c_str());
#endif
			printf(message.c_str(), args...);
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			printf("\033[0m"); //Reseting colour
#endif
		}

		template<typename... Args> void Log_RuntimeError(const std::string& colour, const std::string& message, Args... args)
		{
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			printf("%s", ("\033" + colour).c_str());
#endif
			printf(message.c_str(), args...);
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			printf("\033[0m"); //Reseting colour
#endif
			throw std::runtime_error(message.c_str());
		}
	};
};

#endif