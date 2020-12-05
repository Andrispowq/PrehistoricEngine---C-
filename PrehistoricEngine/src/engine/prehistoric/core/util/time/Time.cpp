#include "engine/prehistoric/core/util/Includes.hpp"
#include "Time.h"


namespace Prehistoric
{
	namespace Time
	{
		double Time::getTime()
		{
			using std::chrono::seconds;
			using std::chrono::duration_cast;

			return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count() / pow(10, 9);
		}

		long long Time::getTimeNanoseconds()
		{
			using std::chrono::nanoseconds;
			using std::chrono::duration_cast;

			return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
		}
	};
};