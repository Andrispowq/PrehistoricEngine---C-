#ifndef TERRAIN_H
#define TERRAIN_H

#include "prehistoric/core/node/Node.h"

#include "TerrainQuadtree.h"

namespace Prehistoric
{
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
};

#endif