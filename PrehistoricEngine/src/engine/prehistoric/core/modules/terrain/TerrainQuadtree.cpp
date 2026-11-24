#include "Includes.hpp"
#include "TerrainQuadtree.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	TerrainQuadtree::TerrainQuadtree(Window* window, Camera* camera, TerrainMaps* maps, AssembledAssetManager* manager)
		: window(window), camera(camera), maps(maps), factory{ maxNodes }
	{
		PipelineHandle pipeline, wireframePipeline, shadowPipeline;
		if (__FrameworkConfig.api == OpenGL)
			CreateOpenGLResources(manager, pipeline, wireframePipeline, shadowPipeline);

		for (int i = 0; i < rootNodes; i++)
		{
			for (int j = 0; j < rootNodes; j++)
			{
				std::stringstream ss;
				ss << "Child ";
				ss << i;
				ss << ", ";
				ss << j;

				AddChild(ss.str(), new(factory) TerrainNode(&factory, window, camera, manager, maps, material, pipeline, wireframePipeline, shadowPipeline,
					{ i / (float)rootNodes, j / (float)rootNodes }, 0, { float(i), float(j) }));
			}
		}

		worldTransform.setScaling({ __TerrainConfig.scaleXZ, __TerrainConfig.scaleY, __TerrainConfig.scaleXZ });
		worldTransform.setPosition(Vector3f{ -__TerrainConfig.scaleXZ / 2.0f, 0, -__TerrainConfig.scaleXZ / 2.0f } + maps->getPosition());
		if (__FrameworkConfig.api == Vulkan) CollectLeaves();
	}

	void TerrainQuadtree::CreateOpenGLResources(AssembledAssetManager* manager, PipelineHandle& pipeline,
		PipelineHandle& wireframePipeline, PipelineHandle& shadowPipeline)
	{
		PatchVertexBuffer* vbo = new GLPatchVertexBuffer(window, generatePatch());
		VertexBufferHandle vboHandle = manager->getAssetManager()->storeVertexBuffer(vbo);
		vboHandle->setFrontFace(FrontFace::COUNTER_CLOCKWISE);

		ShaderHandle shader = manager->getAssetManager()->loadShader(ShaderName::Terrain).value();
		pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vboHandle);
		ShaderHandle wireframeShader = manager->getAssetManager()->loadShader(ShaderName::TerrainWireframe).value();
		wireframePipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, wireframeShader, vboHandle);
		ShaderHandle shadowShader = manager->getAssetManager()->loadShader(ShaderName::TerrainShadow).value();
		shadowPipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shadowShader, vboHandle);

		Material* mat = new Material(manager->getAssetManager(), "TerrainMaterial");
		mat->addTexture("normalMap", maps->getNormalmapHandle());
		mat->addTexture("heightMap", maps->getHeightmapHandle());
		mat->addTexture("splatMap", maps->getSplatmapHandle());

		for (int i = 0; i < maps->getMaterials().size(); i++)
		{
			mat->addInt("subMaterial" + std::to_string(i), maps->getMaterials()[i].handle);
		}

		material = manager->storeMaterial(mat);
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
		if (__FrameworkConfig.api == Vulkan) CollectLeaves();
	}

	void TerrainQuadtree::CollectLeaves()
	{
		leaves.clear();
		for (const auto& child : children)
			static_cast<const TerrainNode*>(child.second.get())->CollectLeaves(leaves);
	}

	std::vector<Vector2f> TerrainQuadtree::generatePatch() const
	{
		return QuadPatchVertices();
	}
};
