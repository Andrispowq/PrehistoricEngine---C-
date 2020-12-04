#pragma once

#include <iostream>
#include <fstream>

#include <algorithm>

#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <sstream>

#include "engine/prehistoric/core/util/log/Log.h"
#include "engine/prehistoric/core/util/time/Time.h"

#include "codes/InputCode.h"

#define MODEL_PATH "res/models/"
#define TEXTURE_PATH "res/textures/"
#define SHADER_PATH "res/shaders/"

template<typename T>
size_t FindElement(T* element, std::vector<T*> list)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		if (list[i] == element)
		{
			return i;
		}
	}

	return 0xFFFFFFFF;
}

constexpr static uint8_t number_of_digits(long long number)
{
	if (number == 0)
		return 1;

	return uint8_t(log10(abs(number))) + 1;
}