#ifndef WATER_NODE_H
#define WATER_NODE_H

#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/common/buffer/PatchVertexBuffer.h"
#include "prehistoric/core/util/math/Math.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"

#include "prehistoric/core/resources/Factory.h"

namespace Prehistoric
{
	struct TexturesData
	{
		Texture* Dy;
		Texture* Dx;
		Texture* Dz;

		Texture* waterReflection = nullptr;
		Texture* waterRefraction = nullptr;
		Texture* normalMap;
		Texture* dudvMap;

		float waterHeight = 0.0f;

		float motion = 0.0f;
		bool underwater = false;
	};

	class WaterNode : public GameObject
	{
	public:
		WaterNode() {}
		WaterNode(Factory<WaterNode>* factory, Window* window, Camera* camera, AssembledAssetManager* manager,
			TexturesData* texData, PipelineHandle pipeline, PipelineHandle wireframePipeline,
			Vector2f location, int lod, Vector2f index, Vector3f position);
		virtual ~WaterNode();

		virtual void PreRender(Renderer* renderer) override;

		void UpdateQuadtree();
		void UpdateChildNodes();

		void AddChildNodes(int lod);
		void RemoveChildNodes();

		void ComputeWorldPosition();

		int getLod() const { return lod; }
		Vector2f getLocation() const { return location; }
		Vector3f getWorldPosition() const { return worldPosition; }
		Vector2f getIndex() const { return index; }
		float getGap() const { return gap; }

		const Transform& getLocalTransform() const { return localTransform; }

		WaterNode(const WaterNode&) = default;

		//Overloaded new and delete comes here:
		void* operator new(size_t size, Factory<WaterNode>& factory)
		{
			void* ptr = factory.allocate();

			if (!ptr)
				throw std::bad_alloc();

			return ptr;
		}

		void* operator new(size_t size)
		{
			void* ptr = malloc(size);

			if (!ptr)
				throw std::bad_alloc();

			return ptr;
		}

		void operator delete(void* ptr, Factory<WaterNode>& factory)
		{
			WaterNode* _ptr = (WaterNode*)ptr;
			_ptr->~WaterNode();
			factory.deallocate(_ptr);
		}

		void operator delete(void* ptr)
		{
			((WaterNode*)ptr)->~WaterNode();
			free(ptr);
		}
	private:
		Factory<WaterNode>* factory;
		AssembledAssetManager* manager;

		Window* window;
		Camera* camera;

		bool leaf;
		int lod;
		Vector2f location;
		Vector3f worldPosition;
		Vector2f index;
		float gap;
		Vector3f position;

		RendererComponent* rendererComponent;
		RendererComponent* wireframeRendererComponent;
		Material* material;

		TexturesData* texData;

		Transform localTransform;
	};
}

#endif