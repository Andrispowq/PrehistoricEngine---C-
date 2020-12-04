#ifndef APPLICATION_H
#define APPLICATION_H

namespace Prehistoric
{
	class PR_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//To be defined in client
	Application* CreateApplication();
};


#endif