#ifndef LOG_H
#define LOG_H

#include "Includes.hpp"
#include "ColourCodes.h"
#include "platform/Platform.h"
#include "prehistoric/core/util/time/Timer.h"
#include <time.h>

//We will ignore messages and warnings in release builds, though errors and runtime errors are useful to have there too
#if defined(PR_DEBUG)
	//Use this macro at the end of a function with its name to profile it
	#define PR_TIME(x)                 ::Prehistoric::Timer t(nullptr, x, false)
	#define PR_PROFILE(x)              ::Prehistoric::Timer t(x, ::Prehistoric::Log::file)
	
	#define PR_LOG_MESSAGE(...)        ::Prehistoric::Log::log(__LINE__, __FILE__, WHITE, __VA_ARGS__)
	#define PR_LOG_WARNING(...)        ::Prehistoric::Log::log(__LINE__, __FILE__, YELLOW, __VA_ARGS__)
	#define PR_LOG_ERROR(...)          ::Prehistoric::Log::log(__LINE__, __FILE__, RED, __VA_ARGS__)
	#define PR_LOG_RUNTIME_ERROR(...)  ::Prehistoric::Log::log_RuntimeError(__LINE__, __FILE__, RED, __VA_ARGS__)
	#define PR_LOG(x, ...)             ::Prehistoric::Log::log(__LINE__, __FILE__, x, __VA_ARGS__)

	#define PR_ASSERT(x, ...) if(!(x)) ::Prehistoric::Log::log_RuntimeError(__LINE__, __FILE__, RED, "Assertion failed: %s\n", __VA_ARGS__)
#else
	#define PR_PROFILE(x)
	
	#define PR_LOG_MESSAGE(...)
	#define PR_LOG_WARNING(...)
	#define PR_LOG_ERROR(...)          ::Prehistoric::Log::log(__LINE__, __FILE__, RED, __VA_ARGS__)
	#define PR_LOG_RUNTIME_ERROR(...)  ::Prehistoric::Log::log_RuntimeError(__LINE__, __FILE__, RED, __VA_ARGS__)
	#define PR_LOG(x, ...)             ::Prehistoric::Log::log(__LINE__, __FILE__, x, __VA_ARGS__)
	#define PR_ASSERT(x, ...)
#endif

namespace Prehistoric
{
	class Log
	{
	public:
		static PR_API std::ofstream* file;

		static PR_API void Init();
		static PR_API void Shutdown();

		template<typename... Args> static PR_API void log_Internal(size_t line, const char* file, const std::string& message, Args... args)
		{
			time_t rawtime;
			struct tm* timeinfo;

			time(&rawtime);
			timeinfo = localtime(&rawtime);
			
			std::stringstream str;
			str << "[" << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec;
#if defined(PR_DEBUG_INCLUDE_FILE_LINE)
			str << ", file: " << file << ", line: " << line;
#endif
			str << "]: ";
			str << message;

			printf(str.str().c_str(), args...);
		}

		template<typename... Args> static PR_API void log(size_t line, const char* file, const std::string& colour, const std::string& message, Args... args)
		{
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			std::cout << "\033" << colour;
#endif
			log_Internal(line, file, message, args...);
		}

		template<typename... Args> static PR_API void log_RuntimeError(size_t line, const char* file, const std::string& colour, const std::string& message, Args... args)
		{
#ifdef PR_COLOURED_LOGGING_FUNCTIONS
			std::cout << "\033" << colour;
#endif
			log_Internal(line, file, message, args...);

			DEBUG_BREAK();
		}
	};
};

#endif