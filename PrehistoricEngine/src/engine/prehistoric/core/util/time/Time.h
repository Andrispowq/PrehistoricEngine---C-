#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Prehistoric
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::high_resolution_clock::time_point ClockTime;
	static ClockTime start = Clock::now();

	namespace Time
	{
		ClockTime getTime();

		double getTimeFromStart();
		long long getTimeFromStartNanoseconds();

		double getDifference(ClockTime start, ClockTime end);
		long long getDifferenceNanoseconds(ClockTime start, ClockTime end);
	};
};

#endif