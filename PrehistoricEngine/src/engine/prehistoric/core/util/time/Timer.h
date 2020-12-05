#ifndef TIMER_H
#define TIMER_H

#include "Time.h"
#include "Includes.hpp"


namespace Prehistoric
{
	class Timer
	{
	public:
		Timer(const std::string& funcName = "unknown");
		virtual ~Timer();
	private:
		double startTime;
		std::string funcName;
	};
};

#endif