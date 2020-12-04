#ifndef TIMER_H
#define TIMER_H

#include "Time.h"
#include "engine/prehistoric/core/util/Includes.hpp"

class Timer
{
public:
	Timer(const std::string& funcName = "unknown");
	virtual ~Timer();
private:
	double startTime;
	std::string funcName;
};

#endif