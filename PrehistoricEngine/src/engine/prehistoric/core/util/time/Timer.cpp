#include "Includes.hpp"
#include "Timer.h"

namespace Prehistoric
{
	Timer::Timer(const std::string& funcName)
	{
		this->funcName = funcName;

		startTime = Time::getTime();
	}

	Timer::~Timer()
	{
		PR_LOG_MESSAGE("Time elapsed executing function %s: %fms\n", funcName.c_str(), (Time::getTime() - startTime) * 1000);
	}
};