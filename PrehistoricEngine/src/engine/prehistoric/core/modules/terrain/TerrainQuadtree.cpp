#include "Includes.hpp"
#include "TerrainQuadtree.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	TerrainQuadtree::TerrainQuadtree(Window* window, Camera* camera, TerrainMaps* maps, AssembledAssetManager* manager)
		: window(window), camera(camera), maps(maps), factory{ 1024 }
	{
		PatchVertexBuffer* vbo = new GLPatchVertexBuffer(window, generatePatch());
		VertexBufferHandle vboHandle = manager->getAssetManager()->storeVertexBuffer(vbo);
		vboHandle->setFrontFace(FrontFace::COUNTER_CLOCKWISE);

		ShaderHandle shader = manager->getAssetManager()->loadShader(ShaderName::Terrain).value();
		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vboHandle);
		ShaderHandle wireframeShader = manager->getAssetManager()->loadShader(ShaderName::TerrainWireframe).value();
		PipelineHandle wireframePipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, wireframeShader, vboHandle);
		ShaderHandle shadowShader = manager->getAssetManager()->loadShader(ShaderName::TerrainShadow).value();
		PipelineHandle shadowPipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shadowShader, vboHandle);

		for (int i = 0; i < rootNodes; i++)
		{
			for (int j = 0; j < rootNodes; j++)
			{
				std::stringstream ss;
				ss << "Child ";
				ss << i;
				ss << ", ";
				ss << j;

				AddChild(ss.str(), new(factory) TerrainNode(&factory, window, camera, manager, maps, pipeline, wireframePipeline, shadowPipeline,
					{ i / (float)rootNodes, j / (float)rootNodes }, 0, { float(i), float(j) }));
			}
		}

		worldTransform.setScaling({ __TerrainConfig.scaleXZ, __TerrainConfig.scaleY, __TerrainConfig.scaleXZ });
		worldTransform.setPosition({ -__TerrainConfig.scaleXZ / 2.0f, 0, -__TerrainConfig.scaleXZ / 2.0f });
	}

	TerrainQuadtree::~TerrainQuadtree()
	{
		//The children will be freed when the Factory is deleted at the end of this destructor but before the Node destructor
		//so we don't want the automatic Node reclaimation to take place, so we clear the list
		for (auto& child : children)
		{
			TerrainNode::operator delete(child.second.release(), factory);
		}

		children.clear();
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
