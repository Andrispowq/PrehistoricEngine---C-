#include "Includes.hpp"
#include "Timer.h"

namespace Prehistoric
{
	Timer::Timer(const std::string& funcName, std::ofstream* file)
	{
		this->funcName = funcName;
		this->file = file;

		startTime = Time::getTime();
	}

	Timer::~Timer()
	{
		auto now = Time::getTime();
		auto diff = Time::getDifference(startTime, now) * 1000.0;
		
		if (file == nullptr)
		{
			PR_LOG_MESSAGE("Time elapsed executing function %s: %fms\n", funcName.c_str(), diff);
		}
		else
		{
			time_t rawtime;
			struct tm* timeinfo;

			time(&rawtime);
			timeinfo = localtime(&rawtime);

			std::stringstream str;
			str << "[" << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec;
			str << "]: ";
			str << "Time elapsed executing function " << funcName;
			str << ": " << diff << "ms\n";
			file->write(str.str().c_str(), str.str().size());
		}
	}
};