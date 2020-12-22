#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Prehistoric
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::steady_clock::time_point ClockTime;
	static const ClockTime start = Clock::now();

	namespace Time
	{
		PR_API double getTime();
		PR_API long long getTimeNanoseconds();
	};
};

#endif