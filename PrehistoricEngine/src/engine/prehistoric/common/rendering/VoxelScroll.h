#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Prehistoric {
    inline float SnapVoxelCenter(float position, float cell) {
        return std::floor(position / cell + 0.5f) * cell;
    }

    struct VoxelScroll {
        std::array<int, 3> delta{}; // New logical cell p retains old cell p + delta.
        bool incremental = false;
        uint64_t dirtyCells = 0;
    };

    inline VoxelScroll PlanVoxelScroll(const std::array<float, 4> &previous,
                                       const std::array<float, 4> &current, unsigned resolution) {
        VoxelScroll plan;
        plan.dirtyCells = uint64_t(resolution) * resolution * resolution;
        if (!resolution || current[3] <= 0 || previous[3] != current[3]) return plan;
        uint64_t retained = 1;
        bool moved = false;
        for (unsigned axis = 0; axis < 3; ++axis) {
            const double cells = (double(current[axis]) - previous[axis]) * resolution / current[3];
            if (!std::isfinite(cells) || std::abs(cells) >= resolution) return plan;
            plan.delta[axis] = int(std::round(cells));
            if (std::abs(cells - plan.delta[axis]) > 0.001) return plan;
            retained *= resolution - unsigned(std::abs(plan.delta[axis]));
            moved |= plan.delta[axis] != 0;
        }
        plan.incremental = moved;
        plan.dirtyCells -= retained;
        return plan;
    }
}