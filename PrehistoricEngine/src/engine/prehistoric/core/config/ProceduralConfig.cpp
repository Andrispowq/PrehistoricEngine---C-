#include "Includes.hpp"
#include "ProceduralConfig.h"
#include "prehistoric/core/modules/terrain/Terrain.h"
#include "prehistoric/core/modules/water/Water.h"

namespace Prehistoric {
    thread_local TerrainConfig *activeTerrainConfig = nullptr;
    thread_local WaterConfig *activeWaterConfig = nullptr;

    ProceduralConfigScope::ProceduralConfigScope(TerrainConfig *terrain, WaterConfig *water)
        : previousTerrain(activeTerrainConfig), previousWater(activeWaterConfig) {
        if (terrain) activeTerrainConfig = terrain;
        if (water) activeWaterConfig = water;
    }

    ProceduralConfigScope::ProceduralConfigScope(const Node *object)
        : ProceduralConfigScope(nullptr, nullptr) {
        for (auto *node = const_cast<Node *>(object); node; node = node->getParent()) {
            if (auto *terrain = dynamic_cast<Terrain *>(node)) {
                activeTerrainConfig = &terrain->GetSettings();
                break;
            }
            if (auto *water = dynamic_cast<Water *>(node)) {
                activeWaterConfig = &water->GetSettings();
                break;
            }
        }
    }

    ProceduralConfigScope::~ProceduralConfigScope() {
        activeTerrainConfig = previousTerrain;
        activeWaterConfig = previousWater;
    }
}