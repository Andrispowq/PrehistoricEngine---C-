#include "Includes.hpp"
#include "WaterNode.h"

#include "WaterQuadtree.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	WaterNode::WaterNode(Factory<WaterNode>* factory, Window* window, Camera* camera, AssembledAssetManager* manager, MaterialHandle waterMaterial, PipelineHandle pipeline, PipelineHandle wireframePipeline, Vector2f location, int lod, Vector2f index, Vector3f position)
		: factory(factory), window(window), camera(camera), manager(manager),
		waterMaterial(waterMaterial), location(location),	lod(lod), index(index), position(position)
	{
		this->gap = 1.0f / float(WaterQuadtree::rootNodes * pow(2, lod));
		
		Vector3f localScaling = { gap, 0, gap };
		Vector3f localPosition = { location.x, 0, location.y };

		localTransform.setScaling(localScaling);
		localTransform.setPosition(localPosition);

		worldTransform.setScaling({ __WaterConfig.scaleXZ, __WaterConfig.displacementScale, __WaterConfig.scaleXZ });
		worldTransform.setPosition(Vector3f{ -__WaterConfig.scaleXZ / 2.0f, 0, -__WaterConfig.scaleXZ / 2.0f } + position);

		if (pipeline.pointer)
		{
			rendererComponent = new RendererComponent(window, manager, pipeline, waterMaterial);
			rendererComponent->setReflection(false);
			wireframeRendererComponent = new RendererComponent(window, manager, wireframePipeline, waterMaterial);
			wireframeRendererComponent->setReflection(false);
			AddComponent(RENDERER_COMPONENT, rendererComponent);
			AddComponent(WIREFRAME_RENDERER_COMPONENT, wireframeRendererComponent);
		}

		ComputeWorldPosition();
		UpdateQuadtree(camera);
	}

	WaterNode::~WaterNode()
	{
		//Ensure custom deletion for the children, as they are allocated differently from the rest of the Nodes
		for (auto& child : children)
		{
			WaterNode::operator delete(child.second.release(), *factory);
		}

		children.clear();
	}

	void WaterNode::PreRender(Renderer* renderer)
	{
		if (!rendererComponent) return;
		if (leaf)
		{
			if (renderer->isWireframeMode())
			{
				wireframeRendererComponent->PreRender(renderer);
			}
			else
			{
				rendererComponent->PreRender(renderer);
			}
		}

		for (auto& child : children)
		{
			child.second->PreRender(renderer);
		}
	}

	void WaterNode::UpdateQuadtree(Camera* activeCamera)
	{
		camera = activeCamera;
		UpdateChildNodes();

		if (children.size() != 0)
		{
			for (auto& child : children)
			{
				((WaterNode*)child.second.get())->UpdateQuadtree(activeCamera);
			}
		}
	}

	void WaterNode::UpdateChildNodes()
	{
		float distance = (camera->getPosition() - worldPosition).length();

		if (size_t(lod) < __WaterConfig.lodRanges.size() && distance < __WaterConfig.lodRanges[lod])
		{
			AddChildNodes(lod + 1);
		}
		else
		{
			RemoveChildNodes();
		}
	}

	void WaterNode::AddChildNodes(int lod)
	{
		if (leaf)
		{
			leaf = false;
		}

		if (children.size() == 0)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					std::stringstream ss;
					ss << "Child ";
					ss << i;
					ss << ", ";
					ss << j;
					ss << ", lod: ";
					ss << lod;

					AddChild(ss.str(), new(*factory) WaterNode(factory, window, camera, manager, waterMaterial, rendererComponent ? rendererComponent->getPipelineHandle() : PipelineHandle{}, wireframeRendererComponent ? wireframeRendererComponent->getPipelineHandle() : PipelineHandle{},
						location + Vector2f(float(i), float(j)) * (gap / 2.f), lod, { float(i), float(j) }, position));
				}
			}
		}
	}

	void WaterNode::RemoveChildNodes()
	{
		if (!leaf)
		{
			leaf = true;
		}

		if (children.size() != 0)
		{
			for (auto& child : children)
			{
				WaterNode::operator delete(child.second.release(), *factory);
			}

			children.clear();
		}
	}

    void WaterNode::CollectLeaves(std::vector<QuadPatch>& leaves) const
    {
        if (leaf)
        {
            float morph = lod ? float(__WaterConfig.lodMorphingAreas[lod-1]) : 0;
            leaves.push_back({{location.x,location.y,gap,morph},{index.x,index.y,float(lod),0}});
            return;
        }
        for (const auto& child : children)
            static_cast<const WaterNode*>(child.second.get())->CollectLeaves(leaves);
    }

    void WaterNode::ComputeWorldPosition()
	{
		Vector2f loc = (location + gap / 2.0f) * __WaterConfig.scaleXZ - __WaterConfig.scaleXZ / 2.0f;
		this->worldPosition = Vector3f(loc.x, 0, loc.y) + position;
	}
}