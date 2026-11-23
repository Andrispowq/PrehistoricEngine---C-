#pragma once

#include "prehistoric/core/modules/atmosphere/AtmosphereCapture.h"

namespace Prehistoric
{
    // Three vec4 camera rays precede the atmosphere block in fullscreen draws.
    constexpr unsigned AtmospherePushOffset = 48;
    static_assert(sizeof(AtmosphereParameters) == 72);
}
