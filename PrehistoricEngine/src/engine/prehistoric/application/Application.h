#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		CoreEngine engine;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif