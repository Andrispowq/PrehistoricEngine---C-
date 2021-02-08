#include "Includes.hpp"
#include "Application.h"

#include "prehistoric/core/events/ApplicationEvent.h"

namespace Prehistoric
{
	Application* Application::instance = nullptr;

	Application::Application()
	{
		if (instance)
			PR_LOG_RUNTIME_ERROR("The application already exists!\n");

		instance = this;

		engineLayer = new CoreEngine();
		engineLayer->getRenderingEngine()->getWindow()->setEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		//Input initialisation
		InputInstance.Init(engineLayer->getRenderingEngine()->getWindow());

		PushLayer(engineLayer);
		
		if (FrameworkConfig::api == OpenGL)
		{
			imGUILayer = new ImGuiLayer();
			PushOverlay(imGUILayer);
			imGUILayer->SetDarkThemeColors();
		}

		frameTime = 1.0 / FrameworkConfig::windowMaxFPS;
		last_fps = FrameworkConfig::windowMaxFPS;
	}

	Application::~Application()
	{
		if (FrameworkConfig::api == OpenGL)
		{
			//Making sure that the ImGUI layer is deleted before the core engine, therefore deleting the GLFWwindow which is needed by ImGUI
			layerStack.PopOverlay(imGUILayer);
			delete imGUILayer;
		}
	}

	void Application::Run()
	{
		if (running)
			return;

		running = true;

		uint32_t frames = 0;
		double frameCounter = 0;

		long long lastTime = Time::getTimeFromStartNanoseconds();
		double unprocessedTime = 0;

		while (running)
		{
			bool render = false;

			long long startTime = Time::getTimeFromStartNanoseconds();
			long long passedTime = startTime - lastTime;
			lastTime = startTime;

			//This piece of code is put here so that whenever the window is iconified, the only code that runs is what is absolutely necessary
			//The code before this check is necessary so that the elapsed time is not tracked, but updated (lastTime = startTime)*
			if (InputInstance.IsPause())
			{
				//Checks for updates, so that we can restore the window later
				InputInstance.Update();
				engineLayer->getRenderingEngine()->getWindow()->Input();

				//Makes sure that this loop only runs a 1000 times per second at most, avoiding unnecessary CPU load
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);

				continue;
			}

			unprocessedTime += passedTime / NANOSECOND;
			frameCounter += passedTime;

			while (unprocessedTime > frameTime)
			{
				render = true;
				unprocessedTime -= frameTime;

				if (engineLayer->getRenderingEngine()->getWindow()->ShouldClose())
				{
					Stop();
					break;
				}

				for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
				{
					float passed = (float)(passedTime / NANOSECOND);
					passed = (float)min(passed, frameTime);
					(*it)->Update(passed);
				}

				if (frameCounter >= NANOSECOND)
				{
					last_fps = frames;
					last_frameTime = passedTime / NANOSECOND * 1000.0f;
					PR_LOG(CYAN, "FPS: %i\n", frames);
					PR_LOG(CYAN, "Delta time: %f ms\n", last_frameTime);
					frames = 0;
					frameCounter = 0;
				}
			}

			if (render && !InputInstance.IsPauseRendering())
			{
				engineLayer->BeginRendering();
				for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
				{
					(*it)->Render();
				}

				if (FrameworkConfig::api == OpenGL)
				{
					imGUILayer->Begin();
					for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
					{
						(*it)->ImGUIRender();
					}
					imGUILayer->End();
				}

				engineLayer->EndRendering();

				frames++;
			}
			else
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
		}
	}

	void Application::Stop()
	{
		if (!running)
			return;
		
		running = false;
	}

	void Application::OnEvent(Event& e)
	{
		for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
		{
			if (e.handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		layerStack.PushOverlay(layer);
		layer->OnAttach();
	}
};
