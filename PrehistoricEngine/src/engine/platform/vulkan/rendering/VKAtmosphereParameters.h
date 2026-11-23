#pragma once

namespace Prehistoric
{
    class Atmosphere;

    struct VKAtmosphereParameters
    {
        // The fullscreen vertex shader uses the first three vec4 slots for camera rays.
        static constexpr unsigned PushOffset = 48;

        float sunPositionRadius[4];
        float sunColourIntensity[4];
        float rayleighMie[4];
        float radiiAndScaleHeights[4];
        float anisotropyAndShift[2];
    };

    static_assert(sizeof(VKAtmosphereParameters) == 72);
    VKAtmosphereParameters GetAtmosphereParameters(const Atmosphere& atmosphere);
}
