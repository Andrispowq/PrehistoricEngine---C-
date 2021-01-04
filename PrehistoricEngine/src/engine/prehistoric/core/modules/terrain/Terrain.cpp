#include "Includes.hpp"
#include "Terrain.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	Terrain::Terrain(Window* window, Camera* camera, ResourceStorage* resourceStorage, const std::string& terrainConfigFile)
		: window(window), camera(camera), maps{ std::make_unique<TerrainMaps>(window, resourceStorage, terrainConfigFile) }
	{
		AddChild("Quadtree", new TerrainQuadtree(window, camera, maps.get(), resourceStorage));
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
