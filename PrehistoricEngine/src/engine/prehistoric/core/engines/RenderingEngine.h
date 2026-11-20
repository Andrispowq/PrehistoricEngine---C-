#ifndef RENDERING_ENGINE_H
#define RENDERING_ENGINE_H

#include "Engine.h"

#include "prehistoric/core/node/movement/FPSCamera.h"
#include "prehistoric/core/node/movement/EditorCamera.h"
#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/config/EngineSettings.h"

#include "prehistoric/common/framework/Window.h"
#include "prehistoric/common/rendering/Renderer.h"

#include "prehistoric/core/node/component/camera/CameraComponent.h"

namespace Prehistoric
{
	struct Statistics
	{
		size_t drawcalls;
		size_t vertexCount;
		size_t indexCount;
	};

	class RenderingEngine : public Engine
	{
	public:
		RenderingEngine();
		virtual ~RenderingEngine() {}

		void Init(AssembledAssetManager* manager);
		void PreRelease();

		virtual void OnEvent(Event& event) override;

		virtual void Update(float delta) override;
		void Render();

		void ChangeCamera(Camera* camera);
		void AddCameraComponent(CameraComponent* component);

		inline Window* getWindow() const { return window.get(); }
		inline Camera* getCamera() const { return primaryCamera; }

		inline Renderer* getRenderer() const { return renderer.get(); }

		static Statistics& getStats() { return statistics; }

		RenderingEngine(const RenderingEngine& engine) = delete;
		RenderingEngine operator=(const RenderingEngine& engine) = delete;
	private:
		void ChangePrimaryCamera(Camera* camera);

	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Camera> defaultCamera;

		std::unique_ptr<Renderer> renderer;

		std::vector<CameraComponent*> cameraComponents;
		Camera* primaryCamera = nullptr;

		static Statistics statistics;
	};
};

#endif