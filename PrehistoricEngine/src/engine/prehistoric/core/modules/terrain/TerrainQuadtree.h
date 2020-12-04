#ifndef TERRAIN_QUADTREE_H
#define TERRAIN_QUADTREE_H

#include "engine/prehistoric/core/node/Node.h"

#include "TerrainNode.h"

#include "engine/platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "engine/platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "engine/platform/opengl/rendering/shaders/terrain/GLTerrainShader.h"
#include "engine/platform/opengl/rendering/shaders/terrain/GLTerrainWireframeShader.h"

#include "engine/platform/opengl/buffer/GLPatchVertexBuffer.h"
#include "engine/platform/vulkan/buffer/VKMeshVertexBuffer.h"

//#include "engine/platform/vulkan/rendering/shaders/terrain/VKTerrainShader.h"
//#include "engine/platform/vulkan/rendering/shaders/terrain/VKTerrainWireframeShader.h"

class TerrainQuadtree : public Node
{
public:
	TerrainQuadtree(Window* window, Camera* camera, TerrainMaps* maps, AssembledAssetManager* manager);
	virtual ~TerrainQuadtree();

	void UpdateQuadtree();
	std::vector<Vector2f> generatePatch() const;

	TerrainQuadtree(const TerrainQuadtree&) = default;
public:
	constexpr static int rootNodes = 8;
private:
	Window* window;
	Camera* camera;

	TerrainMaps* maps;

	Factory<TerrainNode> factory;
};

#endif