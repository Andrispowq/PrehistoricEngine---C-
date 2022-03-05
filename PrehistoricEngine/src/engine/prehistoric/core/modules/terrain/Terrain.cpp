#include "Includes.hpp"
#include "Terrain.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	Terrain::Terrain(Window* window, Camera* camera, AssembledAssetManager* manager, const std::string& terrainConfigFile)
		: window(window), camera(camera), maps{ std::make_unique<TerrainMaps>(window, manager, terrainConfigFile) }
	{
		AddChild("Quadtree", new TerrainQuadtree(window, camera, maps.get(), manager));
	}

	void Terrain::PreRender(Renderer* renderer)
	{
		UpdateQuadtree();
		Node::PreRender(renderer);
	}

	void Terrain::UpdateQuadtree()
	{
		if (camera->isChanged())
		{
			((TerrainQuadtree*)children.at("Quadtree").get())->UpdateQuadtree();
		}
	}
};
