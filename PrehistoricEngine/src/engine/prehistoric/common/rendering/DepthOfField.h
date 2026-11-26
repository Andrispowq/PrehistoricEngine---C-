#pragma once

namespace Prehistoric {
    struct DepthOfFieldSettings {
        bool enabled = false;
        float focusDistance = 30.0f;
        float aperture = 0.5f; // Entrance pupil diameter in world units.

        bool active() const { return enabled && aperture > 0 && focusDistance > 0; }
    };
}