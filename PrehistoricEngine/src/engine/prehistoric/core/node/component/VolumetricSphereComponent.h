#pragma once

#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/node/GameObject.h"

#include <algorithm>

namespace Prehistoric {
    class VolumetricSphereComponent : public Component {
    public:
        VolumetricSphereComponent() {
            type = getStaticComponentType();
        }

        std::unique_ptr<Component> Clone() const override { return std::make_unique<VolumetricSphereComponent>(*this); }

        static ComponentType getStaticComponentType() { return ComponentType::VolumetricSphereComponent; }
        static constexpr const char *Name = "VolumetricSphere";

        void PreRender(Renderer *renderer) override {
            if (radius > 0.0f && density > 0.0f) {
                renderer->RegisterVolumetricSphere({
                    parent->getWorldTransform().getPosition(), radius, density, tint, anisotropy
                });
            }
        }

        float getRadius() const { return radius; }
        float getDensity() const { return density; }
        const Vector3f &getTint() const { return tint; }
        float getAnisotropy() const { return anisotropy; }

        void setRadius(float value) { radius = std::max(0.0f, value); }
        void setDensity(float value) { density = std::max(0.0f, value); }

        void setTint(const Vector3f &value) {
            tint = {
                std::clamp(value.x, 0.0f, 1.0f), std::clamp(value.y, 0.0f, 1.0f),
                std::clamp(value.z, 0.0f, 1.0f)
            };
        }

        void setAnisotropy(float value) { anisotropy = std::clamp(value, -0.95f, 0.95f); }

    private:
        float radius = 10.0f;
        float density = 0.04f;
        Vector3f tint{1.0f};
        float anisotropy = 0.0f;
    };
}