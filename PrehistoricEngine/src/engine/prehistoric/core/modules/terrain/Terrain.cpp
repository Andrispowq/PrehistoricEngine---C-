#include "Includes.hpp"
#include "Terrain.h"
#include "platform/vulkan/rendering/VKTerrain.h"
#include "prehistoric/application/Application.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	Terrain::Terrain(Window* window, Camera* camera, AssembledAssetManager* manager, const std::string& terrainConfigFile)
		: window(window), camera(camera), maps{ std::make_unique<TerrainMaps>(window, manager, terrainConfigFile) }
	{
		lastLodCameraPosition = camera->getPosition();
		quadtree = new TerrainQuadtree(window, camera, maps.get(), manager);
		if (__FrameworkConfig.api == Vulkan)
		{
			CreateVulkanTerrainSurface(*this, window, *maps, manager);
		}
		AddChild("Quadtree", quadtree);
	}

	void Terrain::PreRender(Renderer* renderer)
	{
		UpdateQuadtree();
		GameObject::PreRender(renderer);
	}

	void Terrain::UpdateQuadtree()
	{
		if (quadtree && camera->getPosition() != lastLodCameraPosition)
		{
			quadtree->UpdateQuadtree();
			lastLodCameraPosition = camera->getPosition();
		}
	}
};
