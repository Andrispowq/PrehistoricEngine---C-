#include "Includes.hpp"
#include "TerrainVoxel.h"
#include "Renderer.h"
#include "prehistoric/core/modules/terrain/Terrain.h"
#include "prehistoric/core/model/material/Material.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric {
    static_assert(sizeof(TerrainVoxelParameters) == 224);

    bool TerrainVoxelParameters::operator==(const TerrainVoxelParameters &other) const {
        return model == other.model && grid == other.grid && eye == other.eye && tiling == other.tiling &&
               heights == other.heights && mrot == other.mrot && emission == other.emission &&
               region == other.region && options == other.options;
    }

    bool TerrainVoxelInput::operator==(const TerrainVoxelInput &other) const {
        return parameters == other.parameters && textures == other.textures && textureIds == other.textureIds;
    }

    std::vector<TerrainVoxelInput> CollectTerrainVoxels(const Renderer &renderer,
                                                        const std::array<float, 4> &grid,
                                                        unsigned resolution, bool lighting) {
        std::vector<TerrainVoxelInput> result;
        if (!renderer.voxeliseTerrain || grid[3] <= 0 || !resolution) return result;
        for (auto *terrain: renderer.getTerrains()) {
            auto *maps = terrain->getMaps();
            auto layers = maps->getMaterials();
            if (layers.empty()) continue;
            TerrainVoxelInput input;
            auto &p = input.parameters;
            const auto model = terrain->getWorldTransform().getTransformationMatrix();
            std::copy_n(model.m, 16, p.model.begin());
            p.grid = grid;
            input.textures[0] = maps->getHeightmap();
            input.textures[1] = maps->getNormalmap();
            input.textures[2] = maps->getSplatmap();
            bool detail = false;
            for (unsigned i = 0; i < 3; ++i) {
                auto *layer = layers[std::min(size_t(i), layers.size() - 1)].pointer;
                input.textures[3 + 3 * i] = layer->getTexture(ALBEDO_MAP);
                input.textures[4 + 3 * i] = layer->getTexture(MROT_MAP);
                input.textures[5 + 3 * i] = layer->getTexture(EMISSION_MAP);
                p.tiling[i] = layer->getFloat(HORIZONTAL_SCALE);
                p.heights[i] = layer->getFloat(HEIGHT_SCALE);
                detail |= p.heights[i] != 0;
                const auto value = layer->getVector4f(MROT);
                p.mrot[i] = {value.x, value.y, value.z, value.w};
                p.emission[i] = layer->exists(EMISSION_MAP) ? 1.0f : 0.0f;
            }
            auto eye = renderer.getCamera()->getPosition();
            if (detail) p.eye = {eye.x, eye.y, eye.z, float(__EngineConfig.rendererHighDetailRange)};
            p.heights[3] = __FrameworkConfig.api == OpenGL ? terrain->GetSettings().scaleY : p.model[5];
            // The visible terrain evaluates height after transforming the patch. Match that convention.
            // Clip the regular parameter grid against the volume in XZ, including rotated instances.
            const float a = p.model[0], b = p.model[8], c = p.model[2], d = p.model[10];
            const float det = a * d - b * c;
            if (std::abs(det) < 1e-8f) continue;
            float u0 = 1, v0 = 1, u1 = 0, v1 = 0;
            // Height textures are normalised. Include their XZ displacement for tilted instances.
            for (int i = 0; i < 8; ++i) {
                const float x = grid[0] + (i & 1 ? grid[3] : 0) - p.model[12] - (i & 4 ? p.model[4] : 0);
                const float z = grid[2] + (i & 2 ? grid[3] : 0) - p.model[14] - (i & 4 ? p.model[6] : 0);
                const float u = (d * x - b * z) / det, v = (a * z - c * x) / det;
                u0 = std::min(u0, u);
                u1 = std::max(u1, u);
                v0 = std::min(v0, v);
                v1 = std::max(v1, v);
            }
            const float cell = grid[3] / float(resolution);
            const unsigned nx = std::max(input.textures[0]->getWidth(),
                                         unsigned(std::ceil(2 * std::hypot(a, c) / cell)));
            const unsigned nz = std::max(input.textures[0]->getHeight(),
                                         unsigned(std::ceil(2 * std::hypot(b, d) / cell)));
            if (u1 < 0 || v1 < 0 || u0 > 1 || v0 > 1) continue;
            const int x0 = std::max(0, int(std::floor(u0 * nx)) - 1);
            const int z0 = std::max(0, int(std::floor(v0 * nz)) - 1);
            const int x1 = std::min(int(nx), int(std::ceil(u1 * nx)) + 1);
            const int z1 = std::min(int(nz), int(std::ceil(v1 * nz)) + 1);
            if (x1 <= x0 || z1 <= z0) continue;
            p.region = {float(x0), float(z0), float(x1 - x0), float(z1 - z0)};
            p.options = {float(nx), float(nz), float(resolution), lighting ? 1.0f : 0.0f};
            for (unsigned i = 0; i < input.textures.size(); ++i) input.textureIds[i] = input.textures[i]->getID();
            result.push_back(input);
        }
        return result;
    }
}