#ifndef RENDERING_ENGINE_H
#define RENDERING_ENGINE_H

#include "platform/windows/WindowsWindow.h"
#include "prehistoric/core/node/movement/Camera.h"
#include "prehistoric/core/config/FrameworkConfig.h"

#include "prehistoric/common/buffer/VertexBuffer.h"
#include "prehistoric/common/rendering/pipeline/Pipeline.h"

#include "prehistoric/core/resources/AssetManager.h"

namespace Prehistoric
{
	class GameObject;
	class RenderableComponent;
	class Light;

	class RenderingEngine
	{
	public:
		RenderingEngine();
		virtual ~RenderingEngine();

		void Init(AssembledAssetManager* manager);

		void Update(float delta);
		void Render();

		inline Window* getWindow() const { return window.get(); }
		inline Camera* getCamera() const { return camera.get(); }

		inline Renderer* getRenderer() const { return renderer.get(); }

		RenderingEngine(const RenderingEngine& engine) = delete;
		RenderingEngine operator=(const RenderingEngine& engine) = delete;
	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Camera> camera;

		std::unique_ptr<Renderer> renderer;

		AssembledAssetManager* manager;
	};
};

#endif