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
#include <functional>

#include "prehistoric/core/util/log/Log.h"
#include "prehistoric/core/util/time/Time.h"

#include "platform/Platform.h"

#include "prehistoric/core/util/codes/InputCode.h"

#define BIT(x) (0x1 << x)

#ifdef PR_WINDOWS_64
	#include <Windows.h>
#endif