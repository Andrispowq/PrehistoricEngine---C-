#ifndef RENDERER_H
#define RENDERER_H

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/core/node/movement/Camera.h"

#include "prehistoric/common/rendering/pipeline/Pipeline.h"

namespace Prehistoric
{
	class RenderableComponent;
	class AssembledAssetManager;

	//The light structure used in the internal Forward+ renderer calculations
	struct InternalLight
	{
		Vector4f position;
		Vector4f colour;
		Vector4f intensity_radius;
	};

	struct VisibleIndex
	{
		int index;
	};

	class Renderer
	{
	public:
		Renderer(Window* window, Camera* camera, AssembledAssetManager* manager);
		virtual ~Renderer() {}

		virtual void PrepareRendering() = 0;
		virtual void EndRendering() = 0;

		virtual void Render() = 0;

		virtual CommandBuffer* getDrawCommandBuffer() const { return nullptr; }

		virtual Texture* getOutputTexture() const { return nullptr; }

		void RegisterModel(RenderableComponent* renderable);
		void RegisterLight(Light* light);
		void UnregisterModel(RenderableComponent* renderable);
		void UnregisterLight(Light* light);

		Window* getWindow() const { return window; }
		Camera* getCamera() const { return camera; }

		void setCamera(Camera* camera) { this->camera = camera; }

		inline std::vector<Light*> getLights() const { return lights; }

		inline bool isWireframeMode() const { return wireframeMode; }
		inline void setWireframeMode(bool wire) { this->wireframeMode = wire; }

	protected:
		AssembledAssetManager* manager;

		Window* window;
		Camera* camera;

		//The rendering context
		std::unordered_map<Pipeline*, std::unordered_map<Material*, std::vector<RenderableComponent*>>> models_3d;
		std::unordered_map<Pipeline*, std::unordered_map<Material*, std::vector<RenderableComponent*>>> models_transparency;
		std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_2d;

		std::vector<Light*> lights;

		bool wireframeMode;
	};
};

#endif