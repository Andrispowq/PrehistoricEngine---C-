#ifndef TERRAIN_H
#define TERRAIN_H

#include "prehistoric/core/node/GameObject.h"

#include "TerrainQuadtree.h"

namespace Prehistoric
{
	class Renderer;

	class Terrain : public GameObject
	{
	public:
		Terrain(Window* window, Camera* camera, AssembledAssetManager* manager, const std::string& terrainConfigFile);
		virtual ~Terrain() {}

		void PreRender(Renderer* renderer) override;
		void UpdateQuadtree();

		TerrainMaps* getMaps() const { return maps.get(); }

		Terrain(const Terrain&) = default;
	private:
		Window* window;
		Camera* camera;

		TerrainQuadtree* quadtree = nullptr;

		std::unique_ptr<TerrainMaps> maps;
	};
};

#endif