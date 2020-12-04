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
#elif defined(unix) || defined(__unix) || defined(_unix_)
	#define PR_LINUX
	#error "Linux is currently not supported!"
#elif defined(__APPLE__) || defined(__MACH__)
	#define PR_MAC_OS
	#error "Mac OS is currently not supported!" 
#elif defined(__ANDROID__)
	#define PR_ANDROID
	#error "Android is currently not supported!"
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

#if !defined(PR_NAMESPACE_START)
	#define PR_NAMESPACE_START namespace Prehistoric {
#endif

#endif