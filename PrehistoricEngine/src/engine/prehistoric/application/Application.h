#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/events/Event.h"

#include "prehistoric/core/layers/LayerStack.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/ImGUI/ImGUILayer.h"

namespace Prehistoric
{
	class PR_API Application
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

		static Application& Get() { return *instance; }
	protected:
		LayerStack layerStack;
		bool running = false;
		double frameTime;

		CoreEngine* engineLayer;
		ImGuiLayer* imGUILayer;

		static Application* instance;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif