#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/Engine.h"
#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	private:
		Engine engine;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif