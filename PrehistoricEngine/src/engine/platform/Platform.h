#ifndef PLATFORM_H
#define PLATFORM_H

/*
	This file contains data about the platform we're running the app on,
	the configuration settings we are using, and potentialy the API(s) we are using
*/
#if defined(_WIN32)
	#if defined(_WIN64)
		#define PR_WINDOWS_64
	#else
		#error "Only 64 bit Windows is supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>

	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define PR_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define PR_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
#elif defined(__ANDROID__)
	#define PR_ANDROID
	#error "Android is currently not supported!"
#elif defined(__linux__)
	#define PR_LINUX
	#error "Linux is currently not supported!"
#else
	#define PR_UNKNOWN_OS
	#error "This OS is currently not supported!"
#endif

#if defined(PR_WINDOWS_64)
	#if defined(PR_BUILD_DLL)
		#define PR_API __declspec(dllexport)
	#else
		#define PR_API __declspec(dllimport)
	#endif

	#define LOAD_LIBRARY(x) LoadLibraryA(x)
	#define LOAD_FUNCTION(library, x) GetProcAddress(library, x)
#else
	#error "This OS doesn't support DLLs!"
#endif

#if defined(PR_DEBUG)
	#define PR_ENABLE_DEBUGGING
	#define PR_VK_ENABLE_VALIDATION_LAYERS
	#define PR_COLOURED_LOGGING_FUNCTIONS
#else
	#define PR_DISABLE_DEBUGGING
	#define PR_VK_DISABLE_VALIDATION_LAYERS
#endif

#if defined(PR_WINDOWS_64)
	#define DEBUG_BREAK() __debugbreak()
#endif

#endif