#pragma once

#include <array>
#include <chrono>
#include <optional>
#include <string>

namespace Prehistoric
{
    class Atmosphere;

    struct AtmosphereParameters
    {
        std::array<float, 4> sunPositionRadius;
        std::array<float, 4> sunColourIntensity;
        std::array<float, 4> rayleighMie;
        std::array<float, 4> radiiAndScaleHeights;
        std::array<float, 2> anisotropyAndShift;

        bool Matches(const AtmosphereParameters& other) const;
    };

    AtmosphereParameters GetAtmosphereParameters(const Atmosphere& atmosphere);

    struct EnvironmentCaptureSource
    {
        const Atmosphere* atmosphere = nullptr;
        std::string panorama;
        AtmosphereParameters sky{};
        bool scattering = false;
        float exposure = 0;

        static EnvironmentCaptureSource FromScene(const Atmosphere* atmosphere, float captureExposure);
    };

    class AtmosphereCapture
    {
    public:
        using Clock = std::chrono::steady_clock;
        bool NeedsBake(const EnvironmentCaptureSource& source, float interval, Clock::time_point now = Clock::now()) const;
        void Complete(const EnvironmentCaptureSource& source, Clock::time_point now = Clock::now());

    private:
        std::optional<EnvironmentCaptureSource> previous;
        Clock::time_point completed;
    };
}
