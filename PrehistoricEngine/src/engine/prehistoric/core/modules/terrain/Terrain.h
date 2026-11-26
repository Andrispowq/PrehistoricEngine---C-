#ifndef TERRAIN_H
#define TERRAIN_H

#include "prehistoric/core/node/GameObject.h"

#include "TerrainQuadtree.h"

namespace Prehistoric {
    class Renderer;
    struct TerrainConfig;

    class Terrain : public GameObject {
    public:
        Terrain(Window *window, Camera *camera, AssembledAssetManager *manager, const std::string &terrainConfigFile);

        ~Terrain() override;

        Terrain(Window *window, Camera *camera, AssembledAssetManager *manager,
                const std::string &config, const TerrainConfig &settings);

        void PreRender(Renderer *renderer) override;

        void UpdateQuadtree();

        TerrainQuadtree *getQuadtree() const { return quadtree; }
        TerrainMaps *getMaps() const { return maps.get(); }

        Terrain(const Terrain &) = delete;

        const std::string &GetConfigFile() const { return configFile; }

        std::unique_ptr<GameObject> Clone() const override;

        TerrainConfig &GetSettings() const { return *settings; }

    private:
        std::string configFile;
        std::unique_ptr<TerrainConfig> settings;
        Window *window;
        Camera *camera;
        AssembledAssetManager *manager;
        Vector3f lastLodCameraPosition;

        TerrainQuadtree *quadtree = nullptr;

        std::unique_ptr<TerrainMaps> maps;
    };
};

#endif