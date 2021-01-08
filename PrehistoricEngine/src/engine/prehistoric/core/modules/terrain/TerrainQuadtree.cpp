#include "Includes.hpp"
#include "TerrainQuadtree.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"

namespace Prehistoric
{
	TerrainQuadtree::TerrainQuadtree(Window* window, Camera* camera, TerrainMaps* maps, AssembledAssetManager* manager)
		: window(window), camera(camera), maps(maps), factory{ 512 }
	{
		ShaderHandle shader;
		PipelineHandle pipeline;
		ShaderHandle wireframeShader;
		PipelineHandle wireframePipeline;

		PatchVertexBuffer* vbo = nullptr;

		if (FrameworkConfig::api == OpenGL)
		{
			vbo = new GLPatchVertexBuffer(window, generatePatch());
			VertexBufferHandle vboHandle = manager->getAssetManager()->storeVertexBuffer(vbo);

			shader = manager->getAssetManager()->loadShader("terrain").value();
			wireframeShader = manager->getAssetManager()->loadShader("terrain_wireframe").value();

			pipeline = manager->storePipeline(new GLGraphicsPipeline(window, manager->getAssetManager(), shader, vboHandle));
			wireframePipeline = manager->storePipeline(new GLGraphicsPipeline(window, manager->getAssetManager(), wireframeShader, vboHandle));
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//vbo = new VKPatchVertexBuffer(window, generatePatch());
			//VertexBufferHandle vboHandle = manager->getAssetManager()->storeVertexBuffer(vbo);
			//
			//shader = manager->getAssetManager()->loadShader("terrain").value();
			//wireframeShader = manager->getAssetManager()->loadShader("terrain_wireframe").value();
			//
			//pipeline = manager->storePipeline(new VKGraphicsPipeline(window, manager->getAssetManager(), shader, vboHandle));
			//wireframePipeline = manager->storePipeline(new VKGraphicsPipeline(window, manager->getAssetManager(), wireframeShader, vboHandle));
		}

		for (int i = 0; i < rootNodes; i++)
		{
			for (int j = 0; j < rootNodes; j++)
			{
				std::stringstream ss;
				ss << "Child ";
				ss << i;
				ss << ", ";
				ss << j;

				AddChild(ss.str(), new/*(factory)*/ TerrainNode(&factory, window, camera, manager, maps, pipeline, wireframePipeline,
					{ i / (float)rootNodes, j / (float)rootNodes }, 0, { float(i), float(j) }));
			}
		}

		worldTransform.setScaling({ TerrainConfig::scaleXZ, TerrainConfig::scaleY, TerrainConfig::scaleXZ });
		worldTransform.setPosition({ -TerrainConfig::scaleXZ / 2.0f, 0, -TerrainConfig::scaleXZ / 2.0f });
	}

	TerrainQuadtree::~TerrainQuadtree()
	{
		//The children will be freed when the Factory is deleted at the end of this destructor but before the Node destructor
		//so we don't want the automatic Node reclaimation to take place, so we clear the list
		//children.clear();
	}

	void TerrainQuadtree::UpdateQuadtree()
	{
		for (auto& node : children)
		{
			((TerrainNode*)node.second.get())->UpdateQuadtree();
		}
	}

	std::vector<Vector2f> TerrainQuadtree::generatePatch() const
	{
		std::vector<Vector2f> vertices;
		vertices.reserve(16);

		vertices.push_back({ 0, 0 });
		vertices.push_back({ 0.333f, 0 });
		vertices.push_back({ 0.666f, 0 });
		vertices.push_back({ 1, 0 });

		vertices.push_back({ 0, 0.333f });
		vertices.push_back({ 0.333f, 0.333f });
		vertices.push_back({ 0.666f, 0.333f });
		vertices.push_back({ 1, 0.333f });

		vertices.push_back({ 0, 0.666f });
		vertices.push_back({ 0.333f, 0.666f });
		vertices.push_back({ 0.666f, 0.666f });
		vertices.push_back({ 1, 0.666f });

		vertices.push_back({ 0, 1 });
		vertices.push_back({ 0.333f, 1 });
		vertices.push_back({ 0.666f, 1 });
		vertices.push_back({ 1, 1 });

		return vertices;
	}
};
