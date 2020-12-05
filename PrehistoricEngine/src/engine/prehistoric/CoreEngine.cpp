#include "engine/prehistoric/core/util/Includes.hpp"
#include "CoreEngine.h"

namespace Prehistoric
{
	constexpr static double NANOSECOND = 1000000000;

	CoreEngine::CoreEngine()
		: engine{ std::make_unique<Engine>() }
	{
		running = false;
		frameTime = 1.0 / FrameworkConfig::windowMaxFPS;
	}

	void CoreEngine::Start()
	{
		if (running)
			return;

		running = true;
		Run();
	}

	void CoreEngine::Stop()
	{
		if (!running)
			return;

		running = false;
	}

	/*void CoreEngine::Run()
	{
		long long lastTime = Time::getTimeNanoseconds();
		double amountOfTicks = 60.0;
		double ns = pow(10, 9) / amountOfTicks;
		double delta = 0;
		long timer = Time::getTimeNanoseconds() / pow(10, 6);
		uint32_t updates = 0;
		uint32_t frames = 0;

		while (running)
		{
			long now = Time::getTimeNanoseconds();
			bool render = false;

			delta += (now - lastTime) / ns;
			lastTime = now;

			while (delta >= 1)
			{
				if (engine->getRenderingEngine()->getWindow()->ShouldClose())
				{
					Stop();
					break;
				}

				Input(std::max(delta / NANOSECOND, frameTime));
				Update();

				updates++;
				delta--;
			}

			Render();
			frames++;

			if ((Time::getTimeNanoseconds() / pow(10, 6) - timer) > 1000)
			{
				timer += 1000;
				PR_LOG(CYAN, "FPS: %i, TICKS: %i\n", frames, updates);
				frames = 0;
				updates = 0;
			}
		}
	}*/

	void CoreEngine::Run()
	{
		uint32_t frames = 0;
		double frameCounter = 0;

		long long lastTime = Time::getTimeNanoseconds();
		double unprocessedTime = 0;

		while (running)
		{
			bool render = false;

			long long startTime = Time::getTimeNanoseconds();
			long long passedTime = startTime - lastTime;
			lastTime = startTime;

			unprocessedTime += passedTime / NANOSECOND;
			frameCounter += passedTime;

			while (unprocessedTime > frameTime)
			{
				render = true;
				unprocessedTime -= frameTime;

				Input();

				if (engine->getRenderingEngine()->getWindow()->ShouldClose())
				{
					Stop();
					break;
				}

				Update(frameTime);

				if (frameCounter >= NANOSECOND)
				{
					PR_LOG(CYAN, "FPS: %i\n", frames);
					PR_LOG(CYAN, "Delta time: %f ms\n", frameTime * 1000.0);
					frames = 0;
					frameCounter = 0;
				}
			}

			if (render && !InputInstance.IsPause())
			{
				Render();
				frames++;
			}
			else
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
		}
	}
};
