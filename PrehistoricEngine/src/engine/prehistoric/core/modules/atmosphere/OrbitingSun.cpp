#include "Includes.hpp"
#include "OrbitingSun.h"
#include "prehistoric/core/node/component/light/Light.h"

namespace Prehistoric {
    OrbitingSun::OrbitingSun() {
        SetPhase(0);
        SetPosition({32000, 0, 0});
        AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
    }

    double OrbitingSun::GetPhase() const { return Time::getTimeFromStartNanoseconds() * 1.0e-9 + phaseOffset; }
    void OrbitingSun::SetPhase(double seconds) { phaseOffset = seconds - Time::getTimeFromStartNanoseconds() * 1.0e-9; }

    void OrbitingSun::PreUpdate(CoreEngine *engine) {
        if (!enabled) return;
        double angle = 2.0 * PI * std::fmod(GetPhase(), 120.0) / 120.0;
        SetPosition({float(32000 * std::cos(angle)), float(32000 * std::sin(angle)), 0});
        GameObject::PreUpdate(engine);
    }

    std::unique_ptr<GameObject> OrbitingSun::Clone() const {
        auto copy = std::make_unique<OrbitingSun>();
        copy->components.clear();
        CopyAuthoredStateTo(*copy);
        copy->SetPhase(GetPhase());
        return copy;
    }
}