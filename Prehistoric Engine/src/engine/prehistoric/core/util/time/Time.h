#ifndef TIME_H
#define TIME_H

#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point ClockTime;
static const ClockTime start = Clock::now();

namespace Time
{
	double getTime();
	long long getTimeNanoseconds();
};

#endif