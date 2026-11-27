#include "PrehistoricEngine/src/engine/prehistoric/common/rendering/VoxelScroll.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace Prehistoric;
    const std::array<float, 4> start{0, 0, 0, 32};
    auto positive = PlanVoxelScroll(start, {0.5f, 0, 0, 32}, 64);
    assert(positive.incremental && positive.delta[0] == 1 && positive.dirtyCells == 4096);
    auto diagonal = PlanVoxelScroll(start, {-1, 0.5f, -1.5f, 32}, 64);
    assert(diagonal.incremental && diagonal.delta == (std::array < int, 3 >
    {
        -2, 1, -3
    }
    )
    )
    ;
    assert(diagonal.dirtyCells == 64 * 64 * 64 - 62 * 63 * 61);
    assert(!PlanVoxelScroll(start, {0.1f, 0, 0, 32}, 64).incremental);
    assert(!PlanVoxelScroll(start, {32, 0, 0, 32}, 64).incremental);
    assert(!PlanVoxelScroll(start, {0, 0, 0, 64}, 64).incremental);
    assert(!PlanVoxelScroll(start, start, 64).incremental);
    assert(SnapVoxelCenter(-0.26f, 0.5f) == -0.5f && SnapVoxelCenter(0.24f, 0.5f) == 0);
    std::cout << "Voxel scroll planning passed\n";
}