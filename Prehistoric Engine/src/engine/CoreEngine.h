#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include <chrono>
#include <thread>

#include "engine/prehistoric/core/Engine.h"

#include "engine/config/FrameworkConfig.h"
#include "engine/prehistoric/core/util/time/Time.h"

class CoreEngine
{
public:
	CoreEngine();
	~CoreEngine() {}

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

#endif