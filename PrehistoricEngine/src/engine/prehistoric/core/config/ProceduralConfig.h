#pragma once
namespace Prehistoric {
    class Node;
    struct TerrainConfig;
    struct WaterConfig;
    extern thread_local TerrainConfig *activeTerrainConfig;
    extern thread_local WaterConfig *activeWaterConfig;

    class ProceduralConfigScope {
    public:
        explicit ProceduralConfigScope(const Node *object);

        ProceduralConfigScope(TerrainConfig *terrain, WaterConfig *water);

        ~ProceduralConfigScope();

        ProceduralConfigScope(const ProceduralConfigScope &) = delete;

        ProceduralConfigScope &operator=(const ProceduralConfigScope &) = delete;

    private:
        TerrainConfig *previousTerrain;
        WaterConfig *previousWater;
    };
}