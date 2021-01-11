#include "Includes.hpp"
#include "Time.h"

namespace Prehistoric
{
	namespace Time
	{
		ClockTime Time::getTime()
		{
			return Clock::now();
		}

		double getTimeFromStart()
		{
			return getDifference(start, getTime());
		}

		long long getTimeFromStartNanoseconds()
		{
			return getDifferenceNanoseconds(start, getTime());
		}

		double getDifference(ClockTime start, ClockTime end)
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / pow(10, 9);
		}

		long long getDifferenceNanoseconds(ClockTime start, ClockTime end)
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	};
};