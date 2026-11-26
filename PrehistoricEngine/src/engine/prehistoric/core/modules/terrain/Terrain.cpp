#include "Includes.hpp"
#include "prehistoric/core/scene/world/ProceduralWorld.h"
#include "Terrain.h"
#include "platform/vulkan/rendering/VKTerrain.h"
#include "prehistoric/application/Application.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric {
    Terrain::Terrain(Window *window, Camera *camera, AssembledAssetManager *manager, const std::string &file)
        : Terrain(window, camera, manager, file, __TerrainConfig) {
    }

    Terrain::Terrain(Window *window, Camera *camera, AssembledAssetManager *manager,
                     const std::string &file, const TerrainConfig &config)
        : configFile(file), settings(std::make_unique<TerrainConfig>(config)), window(window), camera(camera),
          manager(manager) {
        ProceduralConfigScope scope(settings.get(), nullptr);
        maps = std::make_unique<TerrainMaps>(window, manager, file);
        lastLodCameraPosition = camera->getPosition();
        quadtree = new TerrainQuadtree(window, camera, maps.get(), manager);
        if (__FrameworkConfig.api == Vulkan) {
            CreateVulkanTerrainSurface(*this, window, *maps, manager);
        }
        quadtree->SetGenerated(true);
        AddChild("Quadtree", quadtree);
        if (__FrameworkConfig.api == OpenGL) worldTransform = quadtree->getWorldTransform();
    }

    Terrain::~Terrain() {
        // Patch destructors use maps and the factory: destroy them before member resources.
        children.clear();
        components.clear();
    }

    std::unique_ptr<GameObject> Terrain::Clone() const {
        auto copy = std::make_unique < Terrain > (window, camera, manager, configFile, *settings);
        CopyProceduralMaterials(*this, *copy);
        CopyAuthoredStateTo(*copy);
        return copy;
    }

    void Terrain::PreRender(Renderer *renderer) {
        if (!enabled) return;
        ProceduralConfigScope scope(settings.get(), nullptr);
        camera = renderer->getCamera();
        if (__FrameworkConfig.api == Vulkan)
            if (auto *surface = GetComponent<RendererComponent>()) UpdateVulkanTerrainLayers(
                *surface->getMaterialHandle().pointer, *maps);
        worldTransform.calculateTransformationMatrix();
        maps->SetInstanceTransform(worldTransform);
        UpdateQuadtree();
        GameObject::PreRender(renderer);
    }

    void Terrain::UpdateQuadtree() {
        ProceduralConfigScope scope(settings.get(), nullptr);
        worldTransform.calculateTransformationMatrix();
        maps->SetInstanceTransform(worldTransform);
        if (quadtree) quadtree->UpdateQuadtree();
    }
}