#ifndef TERRAIN_QUADTREE_H
#define TERRAIN_QUADTREE_H

#include "prehistoric/core/node/Node.h"

#include "TerrainNode.h"

#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "platform/vulkan/rendering/pipeline/VKGraphicsPipeline.h"

#include "platform/opengl/rendering/shaders/terrain/GLTerrainShader.h"
#include "platform/opengl/rendering/shaders/terrain/GLTerrainWireframeShader.h"

#include "platform/opengl/buffer/GLPatchVertexBuffer.h"
#include "platform/vulkan/buffer/VKMeshVertexBuffer.h"

//#include "platform/vulkan/rendering/shaders/terrain/VKTerrainShader.h"
//#include "platform/vulkan/rendering/shaders/terrain/VKTerrainWireframeShader.h"

namespace Prehistoric
{
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
		MaterialHandle material;

		Factory<TerrainNode> factory;
	};
};

#endif