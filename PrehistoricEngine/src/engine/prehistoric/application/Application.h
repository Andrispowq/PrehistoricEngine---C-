#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine/prehistoric/CoreEngine.h"

namespace Prehistoric
{
	class PR_API Application
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