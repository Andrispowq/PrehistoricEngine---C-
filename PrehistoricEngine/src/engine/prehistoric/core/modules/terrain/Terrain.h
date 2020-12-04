#ifndef TERRAIN_H
#define TERRAIN_H

#include "engine/prehistoric/core/node/Node.h"

#include "TerrainQuadtree.h"

class Terrain : public Node
{
public:
	Terrain(Window* window, Camera* camera, AssembledAssetManager* manager);
	virtual ~Terrain() {}

	void PreRender(Renderer* renderer) override;
	void UpdateQuadtree();

	TerrainMaps* getMaps() const { return maps.get(); }

	Terrain(const Terrain&) = default;
private:
	Window* window;
	Camera* camera;

	std::unique_ptr<TerrainMaps> maps;
};

#endif