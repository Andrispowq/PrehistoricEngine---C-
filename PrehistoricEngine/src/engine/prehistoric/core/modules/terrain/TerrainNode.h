#ifndef TERRAIN_NODE_H
#define TERRAIN_NODE_H

#include "prehistoric/core/node/GameObject.h"

#include "TerrainMaps.h"

#include "prehistoric/common/buffer/PatchVertexBuffer.h"
#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "prehistoric/core/resources/Factory.h"

namespace Prehistoric
{
	class PR_API TerrainNode : public GameObject
	{
	public:
		TerrainNode() {}
		TerrainNode(Factory<TerrainNode>* factory, Window* window, Camera* camera, AssembledAssetManager* manager, TerrainMaps* maps,
			size_t pipelineID, size_t wireframePipelineID, const Vector2f& location, int lod, const Vector2f& index);
		virtual ~TerrainNode();

		virtual void PreRender(Renderer* renderer) override;

		void UpdateQuadtree();
		void UpdateChildNodes();

		void AddChildNodes(int lod);
		void RemoveChildNodes();

		void ComputeWorldPosition();
		float getTerrainHeight(const Vector2f& location) const;

		TerrainMaps* getMaps() const { return maps; }

		int getLod() const { return lod; }
		Vector2f getLocation() const { return location; }
		Vector3f getWorldPosition() const { return worldPosition; }
		Vector2f getIndex() const { return index; }
		float getGap() const { return gap; }

		Transform getLocalTransform() const { return localTransform; }

		TerrainNode(const TerrainNode&) = default;

		//Overloaded new and delete comes here:
		/*void* operator new(size_t size, Factory<TerrainNode>& factory)
		{
			return factory.allocate();
		}

		void* operator new(size_t size)
		{
			void* ptr = malloc(size);

			if (!ptr)
				throw std::bad_alloc();

			return ptr;
		}

		void operator delete(void* ptr, Factory<TerrainNode>& factory)
		{
			TerrainNode* _ptr = (TerrainNode*)ptr;
			_ptr->~TerrainNode();
			factory.deallocate(_ptr);
		}

		void operator delete(void* ptr)
		{
			((TerrainNode*)ptr)->~TerrainNode();
			free(ptr);
		}*/
	private:
		Factory<TerrainNode>* factory;
		AssembledAssetManager* manager;

		Window* window;
		Camera* camera;

		TerrainMaps* maps;

		bool leaf;
		int lod;
		Vector2f location;
		Vector3f worldPosition;
		Vector2f index;
		float gap;

		RendererComponent* rendererComponent;
		RendererComponent* wireframeRendererComponent;
		Material* material;

		Transform localTransform;
	};
};

#endif