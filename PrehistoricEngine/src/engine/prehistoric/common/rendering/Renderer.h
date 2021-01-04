#ifndef RENDERER_H
#define RENDERER_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/node/movement/Camera.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "prehistoric/core/resources/ResourceStorage.h"

namespace Prehistoric
{
	class RenderableComponent;

	class Renderer
	{
	public:
		Renderer(Window* window, Camera* camera, ResourceStorage* resourceStorage);
		virtual ~Renderer() {}

		virtual void PrepareRendering() = 0;
		virtual void EndRendering() = 0;

		virtual void Render() = 0;

		virtual CommandBuffer* getDrawCommandBuffer() const { return nullptr; }

		void AddModel(RenderableComponent* renderable);
		void AddLight(Light* light);

		Window* getWindow() const { return window; }
		Camera* getCamera() const { return camera; }

		inline std::vector<Light*> getLights() const { return lights; }

		inline bool isWireframeMode() const { return wireframeMode; }
		inline void setWireframeMode(bool wire) { this->wireframeMode = wire; }

	protected:
		ResourceStorage* resourceStorage;

		Window* window;
		Camera* camera;

		//The rendering context
		std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_3d;
		std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_transparency;
		std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_2d;

		std::vector<Light*> lights;

		bool wireframeMode;
	};
};

#endif