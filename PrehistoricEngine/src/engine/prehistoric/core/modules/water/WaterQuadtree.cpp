#include "Includes.hpp"
#include "WaterQuadtree.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	WaterQuadtree::WaterQuadtree(Window* window, Camera* camera, AssembledAssetManager* manager, TexturesData* texData, Vector3f position)
		: window(window), camera(camera), factory{ 1024 }
	{
		PatchVertexBuffer* vbo = new GLPatchVertexBuffer(window, generatePatch());
		VertexBufferHandle vboHandle = manager->getAssetManager()->storeVertexBuffer(vbo);
		vboHandle->setFrontFace(FrontFace::COUNTER_CLOCKWISE);

		ShaderHandle shader = manager->getAssetManager()->loadShader(ShaderName::Water).value();
		PipelineHandle pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vboHandle);
		ShaderHandle wireframeShader = manager->getAssetManager()->loadShader(ShaderName::WaterWireframe).value();
		PipelineHandle wireframePipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, wireframeShader, vboHandle);

		for (int i = 0; i < rootNodes; i++)
		{
			for (int j = 0; j < rootNodes; j++)
			{
				std::stringstream ss;
				ss << "Child ";
				ss << i;
				ss << ", ";
				ss << j;

				AddChild(ss.str(), new(factory) WaterNode(&factory, window, camera, manager, texData, pipeline, wireframePipeline,
					{ i / (float)rootNodes, j / (float)rootNodes }, 0, { float(i), float(j) }, position));
			}
		}

		worldTransform.setScaling({ __WaterConfig.scaleXZ, __WaterConfig.displacementScale, __WaterConfig.scaleXZ });
		worldTransform.setPosition(Vector3f{ position.x + __WaterConfig.scaleXZ / 2.0f, position.y, position.z + __WaterConfig.scaleXZ / 2.0f });

	}

	WaterQuadtree::~WaterQuadtree()
	{
		//The children will be freed when the Factory is deleted at the end of this destructor but before the Node destructor
		//so we don't want the automatic Node reclaimation to take place, so we clear the list
		for (auto& child : children)
		{
			WaterNode::operator delete(child.second.release(), factory);
		}

		children.clear();
	}

	void WaterQuadtree::UpdateQuadtree()
	{
		for (auto& node : children)
		{
			((WaterNode*)node.second.get())->UpdateQuadtree();
		}
	}

	std::vector<Vector2f> WaterQuadtree::generatePatch() const
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
}