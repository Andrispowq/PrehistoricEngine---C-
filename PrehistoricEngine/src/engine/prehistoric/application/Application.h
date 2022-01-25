#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/events/Event.h"

#include "prehistoric/core/layers/LayerStack.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/ImGUI/ImGUILayer.h"

namespace Prehistoric
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Stop();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		CoreEngine* getEngineLayer() { return engineLayer; }
		ImGuiLayer* getImGuiLayer() { return imGUILayer; }

		int getLastFPS() const { return last_fps; }
		double getFrameTime() const { return last_frameTime; }

		static Application& Get() { return *instance; }

	protected:
		LayerStack layerStack;
		bool running = false;
		double frameTime;
		double last_frameTime;
		int last_fps;

		std::unique_ptr<Window> window;

		CoreEngine* engineLayer;
		ImGuiLayer* imGUILayer;

		static Application* instance;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif