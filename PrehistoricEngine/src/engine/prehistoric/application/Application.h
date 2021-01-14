#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/events/Event.h"

#include "prehistoric/core/layers/LayerStack.h"

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

		//ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		static Application& Get() { return *instance; }
	protected:
		LayerStack layerStack;
		bool running = false;
		double frameTime;

		CoreEngine* engineLayer;

		static Application* instance;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif