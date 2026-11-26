#pragma once

namespace Prehistoric {
    struct AmbientOcclusionSettings {
        bool enabled = false;
        float radius = 2.0f; // World units.
        float strength = 1.0f;

        bool active() const { return enabled && radius > 0.0f && strength > 0.0f; }
    };
}