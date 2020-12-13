#ifndef ENGINE_H
#define ENGINE_H

#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	class Engine
	{
	public:
		Engine() {}
		virtual ~Engine() {}

		virtual void OnEvent(Event& event) = 0;
		virtual void Update(float deltaTime) = 0;

		Engine(const Engine& engine) = delete;
		Engine operator=(const Engine& engine) = delete;
	};
};

#endif