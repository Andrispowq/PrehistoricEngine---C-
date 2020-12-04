#ifndef UTIL_H
#define UTIL_H

#include "Includes.hpp"

namespace Util
{
	std::vector<std::string> Split(const std::string& s, char delim);
	std::vector<std::string> RemoveEmptyStrings(const std::vector<std::string>& vec);
};

#endif