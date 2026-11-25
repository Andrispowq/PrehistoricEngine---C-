#include "Includes.hpp"
#include "Atmosphere.h"
#include "AtmosphereCapture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
    AtmosphereParameters GetAtmosphereParameters(const Atmosphere& atmosphere)
    {
        AtmosphereParameters sky{};
        const auto& settings = __AtmosphereConfig;
        auto pack = [](std::array<float, 4>& values, const Vector3f& xyz, float w)
        {
            values[0] = xyz.x;
            values[1] = xyz.y;
            values[2] = xyz.z;
            values[3] = w;
        };
        Vector3f direction = atmosphere.getSunPosition();
        
        if (direction.lengthSquared() > 0) direction = direction.normalise();
        pack(sky.sunPositionRadius, direction, settings.sunRadius);
        pack(sky.sunColourIntensity, settings.sunColour, settings.sunIntensity);

        pack(sky.rayleighMie, settings.rayleigh, settings.mie);
        sky.radiiAndScaleHeights[0] = settings.planetRadius;
        sky.radiiAndScaleHeights[1] = settings.atmosphereRadius;
        sky.radiiAndScaleHeights[2] = settings.rayleighHeightScale;
        sky.radiiAndScaleHeights[3] = settings.mieHeightScale;
        sky.anisotropyAndShift[0] = settings.mieDirection;
        sky.anisotropyAndShift[1] = settings.horizontalVerticalShift;

        return sky;
    }

    bool AtmosphereParameters::Matches(const AtmosphereParameters& other) const
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            if (std::abs(sunPositionRadius[i] - other.sunPositionRadius[i]) > 0.00001f) return false;
        }

        return sunPositionRadius[3] == other.sunPositionRadius[3] && sunColourIntensity == other.sunColourIntensity &&
            rayleighMie == other.rayleighMie && radiiAndScaleHeights == other.radiiAndScaleHeights &&
            anisotropyAndShift == other.anisotropyAndShift;
    }

    EnvironmentCaptureSource EnvironmentCaptureSource::FromScene(const Atmosphere* atmosphere, float captureExposure,
                                                               const std::string& scenePanorama)
    {
        EnvironmentCaptureSource source;
        source.atmosphere = atmosphere;
        if (!atmosphere) source.panorama = scenePanorama.empty() ? __EnvironmentMapConfig.environmentMapLocation : scenePanorama;
        source.scattering = atmosphere && __AtmosphereConfig.scatteringEnabled;

        if (source.scattering)
        {
            source.sky = GetAtmosphereParameters(*atmosphere);
            source.exposure = captureExposure;
        }

        return source;
    }

    bool AtmosphereCapture::NeedsBake(const EnvironmentCaptureSource& source, float interval, Clock::time_point now) const
    {
        if (!previous || source.atmosphere != previous->atmosphere || source.panorama != previous->panorama ||
            source.scattering != previous->scattering || source.includeSunDisk != previous->includeSunDisk)
        {
            return true;
        }

        if (source.sky.Matches(previous->sky) && source.exposure == previous->exposure) 
            return false;
        return now - completed >= std::chrono::duration<float>(interval);
    }

    void AtmosphereCapture::Complete(const EnvironmentCaptureSource& source, Clock::time_point now)
    {
        previous = source;
        completed = now;
        PR_LOG_MESSAGE("Baked environment lighting from %s.\n", source.atmosphere ? "atmosphere" : "panorama");
    }

    void Atmosphere::PreRender(Renderer* renderer)
    {
        if (!isEnabled()) return;
        if (sun) renderer->RegisterAtmosphere(*this);
        GameObject::PreRender(renderer);
    }

    namespace
    {
        class OrbitingSun final : public GameObject
        {
        public:
            OrbitingSun()
            {
                UpdatePosition(0.0);
                AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
            }

            void PreUpdate(CoreEngine* engine) override
            {
                if (!isEnabled()) return;
                const auto now = Clock::now();
                if (!sunrise) sunrise = now;
                UpdatePosition(std::chrono::duration<double>(now - *sunrise).count());
                GameObject::PreUpdate(engine);
            }

        private:
            using Clock = std::chrono::steady_clock;
            std::optional<Clock::time_point> sunrise;

            void UpdatePosition(double elapsed)
            {
                constexpr double period = 120.0;
                constexpr double distance = 32000.0;
                double angle = 2.0 * PI * std::fmod(elapsed, period) / period;
                SetPosition({float(distance * std::cos(angle)), float(distance * std::sin(angle)), 0});
            }
        };
    }

    std::unique_ptr<GameObject> Atmosphere::CreateSun()
    {
        return std::make_unique<OrbitingSun>();
    }
}
