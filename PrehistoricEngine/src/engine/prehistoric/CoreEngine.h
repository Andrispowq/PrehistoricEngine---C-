#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include <chrono>
#include <thread>

#include "core/Engine.h"

#include "prehistoric/core/config/FrameworkConfig.h"
#include "core/util/time/Time.h"

namespace Prehistoric
{
	class CoreEngine
	{
	public:
		CoreEngine();
		~CoreEngine() {}

		Engine* getEngine() { return engine.get(); }

		void Start();
	private:
		void Stop();

		void Run();

		void Input() { engine->Input(); }
		void Update(double passedTime) { engine->Update(static_cast<float>(passedTime)); }
		void Render() { engine->Render(); }

		CoreEngine(const CoreEngine& engine) = delete;
		CoreEngine operator=(const CoreEngine& engine) = delete;
	private:
		bool running;
		double frameTime;

		std::unique_ptr<Engine> engine;
	};
};

#endif