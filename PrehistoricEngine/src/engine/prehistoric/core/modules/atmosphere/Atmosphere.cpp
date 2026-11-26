#include "Includes.hpp"
#include "Atmosphere.h"
#include "OrbitingSun.h"
#include "EnvironmentMap.h"
#include "AtmosphereCapture.h"
#include "prehistoric/application/Application.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric {
    void Atmosphere::setSun(Light *value) {
        if (!value || !value->getParent() || (GetScene() && value->getParent()->GetScene() != GetScene()))
            throw std::runtime_error("Atmosphere sun must belong to its world");
        sun = value;
        sun->setIlluminate(false);
    }

    AtmosphereParameters GetAtmosphereParameters(const Atmosphere &atmosphere) {
        AtmosphereParameters sky{};
        const auto &settings = __AtmosphereConfig;
        auto pack = [](std::array<float, 4> &values, const Vector3f &xyz, float w) {
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

    bool AtmosphereParameters::Matches(const AtmosphereParameters &other) const {
        for (unsigned i = 0; i < 3; ++i) {
            if (std::abs(sunPositionRadius[i] - other.sunPositionRadius[i]) > 0.00001f) return false;
        }

        return sunPositionRadius[3] == other.sunPositionRadius[3] && sunColourIntensity == other.sunColourIntensity &&
               rayleighMie == other.rayleighMie && radiiAndScaleHeights == other.radiiAndScaleHeights &&
               anisotropyAndShift == other.anisotropyAndShift;
    }

    EnvironmentCaptureSource EnvironmentCaptureSource::FromScene(const Atmosphere *atmosphere, float captureExposure,
                                                                 const EnvironmentMap *environment) {
        EnvironmentCaptureSource source;
        source.atmosphere = atmosphere;
        if (bool(atmosphere) == bool(environment))
            throw std::runtime_error("Rendering requires exactly one sky source");
        if (environment) {
            source.panorama = environment->GetAsset();
            source.worldToEnvironment = Matrix4f::Rotation(environment->getWorldTransform().getRotation()).Invert();
        }
        source.scattering = atmosphere != nullptr;

        if (source.scattering) {
            source.sky = GetAtmosphereParameters(*atmosphere);
            source.exposure = captureExposure;
        }

        return source;
    }

    bool AtmosphereCapture::NeedsBake(const EnvironmentCaptureSource &source, float interval,
                                      Clock::time_point now) const {
        if (!previous || source.atmosphere != previous->atmosphere || source.panorama != previous->panorama ||
            source.scattering != previous->scattering || source.includeSunDisk != previous->includeSunDisk ||
            !(source.worldToEnvironment == previous->worldToEnvironment)) {
            return true;
        }

        if (source.sky.Matches(previous->sky) && source.exposure == previous->exposure)
            return false;
        return now - completed >= std::chrono::duration<float>(interval);
    }

    void AtmosphereCapture::Complete(const EnvironmentCaptureSource &source, Clock::time_point now) {
        previous = source;
        completed = now;
        PR_LOG_MESSAGE("Baked environment lighting from %s.\n", source.atmosphere ? "atmosphere" : "panorama");
    }

    void Atmosphere::PreRender(Renderer *renderer) {
        if (!isEnabled()) return;
        if (sun) renderer->RegisterAtmosphere(*this);
        GameObject::PreRender(renderer);
    }

    std::unique_ptr<GameObject> Atmosphere::CreateSun() {
        return std::make_unique<OrbitingSun>();
    }
}