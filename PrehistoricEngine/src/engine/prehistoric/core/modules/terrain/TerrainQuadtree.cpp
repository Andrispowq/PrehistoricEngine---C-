#include "Includes.hpp"
#include "TerrainQuadtree.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	TerrainQuadtree::TerrainQuadtree(Window* window, Camera* camera, TerrainMaps* maps, ResourceStorage* resourceStorage)
		: window(window), camera(camera), maps(maps), factory{ 512 }
	{
		ShaderHandle shader;
		PipelineHandle pipeline;
		ShaderHandle wireframeShader;
		PipelineHandle wireframePipeline;

		PatchVertexBuffer* vbo = nullptr;

		if (FrameworkConfig::api == OpenGL)
		{
			vbo = new GLPatchVertexBuffer(generatePatch());
			VertexBufferHandle vboHandle = resourceStorage->storeVertexBuffer(vbo);

			shader = resourceStorage->loadShader("terrain").value();
			wireframeShader = resourceStorage->loadShader("terrain_wireframe").value();

			pipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, shader, vboHandle));
			wireframePipeline = resourceStorage->storePipeline(new GLGraphicsPipeline(window, resourceStorage, wireframeShader, vboHandle));
		}
		else if (FrameworkConfig::api == Vulkan)
		{
			//vbo = new VKPatchVertexBuffer(generatePatch());
			//VertexBufferHandle vboHandle = resourceStorage->storeVertexBuffer(vbo);
			//
			//shader = resourceStorage->loadShader("terrain").value();
			//wireframeShader = resourceStorage->loadShader("terrain_wireframe").value();
			//
			//pipeline = resourceStorage->storePipeline(new VKGraphicsPipeline(window, resourceStorage, shader, vboHandle));
			//wireframePipeline = resourceStorage->storePipeline(new VKGraphicsPipeline(window, resourceStorage, wireframeShader, vboHandle));
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

				AddChild(ss.str(), new/*(factory)*/ TerrainNode(&factory, window, camera, resourceStorage, maps, pipeline, wireframePipeline,
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
