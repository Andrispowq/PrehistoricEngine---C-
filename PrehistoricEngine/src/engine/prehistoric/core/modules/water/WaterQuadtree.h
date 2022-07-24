#ifndef WATER_QUADTREE_H
#define WATER_QUADTREE_H

#include "prehistoric/core/node/Node.h"

#include "WaterNode.h"

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
	class WaterQuadtree : public Node
	{
	public:
		WaterQuadtree(Window* window, Camera* camera, AssembledAssetManager* manager, TexturesData* texData, Vector3f position);
		virtual ~WaterQuadtree();

		void UpdateQuadtree();
		std::vector<Vector2f> generatePatch() const;

		WaterQuadtree(const WaterQuadtree&) = default;
	public:
		constexpr static int rootNodes = 8;
	private:
		Window* window;
		Camera* camera;

		Factory<WaterNode> factory;
	};
}

#endif