#ifndef UTIL_H
#define UTIL_H

#include "Includes.hpp"

namespace Prehistoric
{
	namespace Util
	{
		PR_API std::vector<std::string> Split(const std::string& s, char delim);
		PR_API std::vector<std::string> RemoveEmptyStrings(const std::vector<std::string>& vec);
	};
};

#endif