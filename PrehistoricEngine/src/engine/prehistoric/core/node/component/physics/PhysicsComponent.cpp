#include "Includes.hpp"
#include "PhysicsComponent.h"

#include "prehistoric/core/CoreEngine.h"

namespace Prehistoric
{
	PhysicsComponent::PhysicsComponent(float mass, bool affectedByGravity)
		: mass(mass), affectedByGravity(affectedByGravity)
	{
	}

	PhysicsComponent::~PhysicsComponent()
	{
	}

	void PhysicsComponent::PreUpdate(CoreEngine* engine)
	{
		engine->getPhysicsEngine()->addPhysicsComponent(this);
	}

	void PhysicsComponent::Update(float delta)
	{
		if (affectedByGravity)
		{
			AddForce(gravity * mass * delta);
		}

		velocity += acceleration * delta;
		parent->Move(velocity * delta);

		if (collider)
		{
			collider->Update(parent);
		}
	}

	void PhysicsComponent::AddForce(Vector3f force)
	{
		acceleration += (force / mass);
	}
};
