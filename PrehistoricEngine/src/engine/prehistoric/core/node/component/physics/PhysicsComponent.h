#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"
#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/core/node/component/physics/colliders/Collider.h"

namespace Prehistoric
{
	/*
	*	This physics component class relies on some input values pulled from the real world, so the type of metrics is important:
	* 
	*	Direction is where velocity and acceleration are applied
	*	Velocity and acceleration is given in metres per second (squared)
	*	Mass is given in kilograms
	* 
	*	Force is given in Newton (metres * kilograms per seconds squared)
	*/

	class PhysicsComponent : public Component
	{
	public:
		PhysicsComponent(float mass = 1.0f, bool affectedByGravity = false);
		~PhysicsComponent();

		virtual void PreUpdate(CoreEngine* engine);
		void Update(float delta);

		void AddForce(Vector3f force);
		Vector3f GetForce() const { return acceleration * mass; }
		Vector3f GetMomentum() const { return velocity * mass; }

		Vector3f getVelocity() const { return velocity; }
		Vector3f getAcceleration() const { return acceleration; }
		float getMass() const { return mass; }
		bool isAffectedByGravity() const { return affectedByGravity; }

		Collider* getCollider() { return collider; }

		void setVeloctiy(Vector3f velocity) { this->velocity = velocity; }
		void setAcceleration(Vector3f acceleration) { this->acceleration = acceleration; }
		void setMass(float mass) { this->mass = mass; }
		void setAffectedByGravity(bool affected) { this->affectedByGravity = affected; }

		void setCollider(Collider* collider) { this->collider = collider; }

		static ComponentType getStaticComponentType() { return ComponentType::PhysicsComponent; }

	private:
		Vector3f velocity = 0.0f;
		Vector3f acceleration = 0.0f;
		float mass = 1.0f;
		bool affectedByGravity = false;

		Collider* collider = nullptr;
	};
};

#endif