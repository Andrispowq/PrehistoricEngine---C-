#pragma once

#include <array>
#include <vector>

namespace Prehistoric {
    class Renderer;
    class Texture;

    // Matches shared/voxel/terrain.glsl. Arrays avoid host ABI/vector padding.
    struct TerrainVoxelParameters {
        std::array<float, 16> model{};
        std::array<float, 4> grid{}, eye{}, tiling{}, heights{};
        std::array<std::array < float, 4>
        ,
        3
        >
        mrot {
        };
        std::array<float, 4> emission{}, region{}, options{};

        bool operator==(const TerrainVoxelParameters &other) const;
    };

    struct TerrainVoxelInput {
        TerrainVoxelParameters parameters;
        // Height, normal, splat, then albedo/MROT/emission for each layer.
        std::array<Texture *, 12> textures{};
        std::array<unsigned, 12> textureIds{};

        bool operator==(const TerrainVoxelInput &other) const;
    };

    std::vector<TerrainVoxelInput> CollectTerrainVoxels(const Renderer &renderer,
                                                        const std::array<float, 4> &grid,
                                                        unsigned resolution, bool lighting);
}