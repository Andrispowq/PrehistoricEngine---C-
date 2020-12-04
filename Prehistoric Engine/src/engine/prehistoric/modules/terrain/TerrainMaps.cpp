#include "engine/prehistoric/core/util/Includes.hpp"
#include "TerrainMaps.h"

#include "engine/prehistoric/resources/AssembledAssetManager.h"

TerrainMaps::TerrainMaps(Window* window, AssembledAssetManager* manager)
{
	this->manager = manager;

	heightmapID = manager->getAssetManager()->getResource<Texture>(TerrainConfig::heightmap);
	manager->getAssetManager()->addReference<Texture>(heightmapID);
	this->heightmap = manager->getAssetManager()->getResourceByID<Texture>(heightmapID);

 	this->normalmapRendererComponent = new NormalMapRenderer(window, manager, 60, heightmap->getWidth());
	normalmapRendererComponent->Render(heightmap);
	this->normalmap = normalmapRendererComponent->getNormalmap();

	this->splatmapRendererComponent = new SplatMapRenderer(window, manager, normalmap->getWidth());
	splatmapRendererComponent->Render(normalmap);
	this->splatmap = splatmapRendererComponent->getSplatmap();

	this->query = new TerrainHeightsQuery(window, manager, heightmap->getWidth());
	query->Query(heightmap);
	this->heights = query->getHeights();
}

TerrainMaps::~TerrainMaps()
{
	manager->getAssetManager()->removeReference<Texture>(heightmapID);

	delete normalmapRendererComponent;
	delete splatmapRendererComponent;
	delete query;
}
