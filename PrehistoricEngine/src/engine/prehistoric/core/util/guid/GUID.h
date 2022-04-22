#ifndef GUID_H
#define GUID_H

#include "Includes.hpp"

namespace Prehistoric
{
	typedef uint64_t GUID;

	GUID GenerateGUID(std::string name);
};

#endif