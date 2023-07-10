#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "Engine.h"

#include "prehistoric/core/resources/AssembledAssetManager.h"
#include "prehistoric/core/config/EngineSettings.h"

#include "prehistoric/core/node/component/physics/PhysicsComponent.h"

namespace Prehistoric
{
	const static Vector3f gravity = Vector3f(0, -9.80665f, 0);

	class PhysicsEngine : public Engine
	{
	public:
		PhysicsEngine();
		virtual ~PhysicsEngine() {}

		virtual void OnEvent(Event& event) override;
		virtual void Update(float delta) override;

		void addPhysicsComponent(PhysicsComponent* physicsComponent);

		PhysicsEngine(const PhysicsEngine& engine) = delete;
		PhysicsEngine operator=(const PhysicsEngine& engine) = delete;
	private:
		std::vector<PhysicsComponent*> physicsComponents;
		bool simulate = false;
	};
};

#endif