#pragma once

#include "prehistoric/core/util/math/Math.h"
#include <array>

namespace Prehistoric {
    class Renderer;
    class Light;

    struct ShadowCascades {
        static constexpr unsigned Count = 7;
        static constexpr unsigned Resolution = 4096;
        static constexpr float BlendFraction = 0.1f;

        ShadowCascades();

        void Update(const Matrix4f &view, float fov, float aspect, Vector3f lightDir, unsigned resolution);

        static const Light *FindSun(const Renderer &renderer);

        std::array<float, Count - 1> splits{};
        std::array<Matrix4f, Count> matrices;
    };
}