#pragma once

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric {
    class Material;
    class Terrain;
    class TerrainMaps;
    class Window;
    class AssembledAssetManager;

    VertexBufferHandle CreateVulkanQuadPatch(Window *window, AssembledAssetManager *manager);

    void BakeVulkanTerrainMaps(Window *window, AssembledAssetManager *manager, TextureHandle height,
                               TextureHandle &normal, TextureHandle &splat, std::vector<float> &heights);

    void UpdateVulkanTerrainLayers(Material &target, TerrainMaps &maps);

    void CreateVulkanTerrainSurface(Terrain &terrain, Window *window, TerrainMaps &maps,
                                    AssembledAssetManager *manager);
}