#ifndef APPLICATION_H
#define APPLICATION_H

#include "prehistoric/core/Engine.h"
#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	class PR_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		Engine engine;
	};

	//To be defined in client
	Application* CreateApplication();
};

#endif