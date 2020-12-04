#include "engine/prehistoric/core/util/Includes.hpp"
#include "Terrain.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

Terrain::Terrain(Window* window, Camera* camera, AssembledAssetManager* manager)
	: window(window), camera(camera), maps{ std::make_unique<TerrainMaps>(window, manager) }
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
