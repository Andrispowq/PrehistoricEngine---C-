#include "Includes.hpp"
#include "GUID.h"

namespace Prehistoric
{
	static long long GetTime()
	{
		return Time::getTimeFromStartNanoseconds();
	}

	static uint64_t HashName(std::string name)
	{
		uint64_t hash = 0;
		
		char* namePtr = &name[0];
		for (size_t i = 0; i < name.length(); i++)
		{
			hash = ((hash << 63) | (hash >> 1)) + ((uint64_t)namePtr[i] << ((i % 8) * 8));
		}

		return hash;
	}

	GUID GenerateGUID(std::string name)
	{
		GUID id = 0;
		id |= GetTime();
		id |= HashName(name);
		return id;
	}
};