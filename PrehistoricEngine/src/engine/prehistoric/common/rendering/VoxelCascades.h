#pragma once
#include "VoxelScroll.h"
#include <array>
#include <cstddef>

namespace Prehistoric {
    // GPU snapshot: metadata, four bounds and four layouts, then packed grid chains.
    struct VoxelCascadeSnapshot {
        std::array<uint32_t, 4> meta{};
        std::array<std::array < float, 4>
        ,
        4
        >
        bounds {
        };
        std::array<std::array<uint32_t, 4>, 4> grids{};
    };

    static_assert(sizeof(VoxelCascadeSnapshot) == 144);

    inline uint32_t VoxelPyramid(unsigned n) { return (8u * n * n * n - 1u) / 7u; }

    inline size_t MakeVoxelCascades(VoxelCascadeSnapshot &snapshot, unsigned nearResolution,
                                    unsigned count, const std::array<float, 3> &center,
                                    float width, bool directional) {
        snapshot = {};
        snapshot.meta = {std::clamp(count, 1u, 4u), unsigned(directional), 0, 0};
        uint32_t words = sizeof(snapshot) / 4;
        for (unsigned i = 0; i < snapshot.meta[0]; ++i) {
            const unsigned n = i ? 64 : nearResolution;
            const float extent = width * float(1u << i);
            const float cell = extent / n;
            snapshot.bounds[i] = {
                SnapVoxelCenter(center[0], cell) - extent / 2,
                SnapVoxelCenter(center[1], cell) - extent / 2,
                SnapVoxelCenter(center[2], cell) - extent / 2, extent
            };
            const auto pyramid = VoxelPyramid(n);
            snapshot.grids[i] = {n, words, words + pyramid, words + 3 * pyramid};
            words += 3 * pyramid + 12 * (pyramid - n * n * n);
        }
        return size_t(words) * 4;
    }
}