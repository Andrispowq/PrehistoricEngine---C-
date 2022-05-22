#include "Includes.hpp"
#include "PhysicsEngine.h"

namespace Prehistoric
{
	PhysicsEngine::PhysicsEngine()
	{
	}

	void PhysicsEngine::OnEvent(Event& event)
	{
	}

	void PhysicsEngine::Update(float delta)
	{
		for (size_t i = 0; i < physicsComponents.size(); i++)
		{
			auto elem = physicsComponents[i];
			elem->Update(delta);

			for (size_t j = i + 1; j < physicsComponents.size(); j++)
			{
				auto elem1 = physicsComponents[j];

				HitInfo info;
				if (elem->getCollider()->Collide(elem1->getCollider(), info))
				{
					Vector3f momentum0 = elem->GetMomentum();
					Vector3f momentum1 = elem1->GetMomentum();
					Vector3f momentum = momentum0;
					if (momentum == 0.0f)
					{
						momentum = momentum1 * -1; //Should be good enough
					}

					Vector3f direction0 = info.direction.normalise();
					Vector3f direction1 = direction0.reflect(momentum.normalise());

					Vector3f new_momentum0 = (momentum0 == 0.0f) ? 0.0f : momentum0.reflect(direction1);
					Vector3f new_momentum1 = (momentum1 == 0.0f) ? 0.0f : momentum1.reflect(direction0);
					float momentum_avg = (new_momentum0.length() + new_momentum1.length()) / 2;
					new_momentum0 = ((new_momentum0 == 0.0f) ? momentum1.normalise() : new_momentum0.normalise()) * momentum_avg;
					new_momentum1 = ((new_momentum1 == 0.0f) ? momentum0.normalise() : new_momentum1.normalise()) * momentum_avg;

					Vector3f force0 = elem->GetForce();
					Vector3f force1 = elem1->GetForce();
					Vector3f new_force0 = (force0 == 0.0f) ? 0.0f : force0.reflect(direction1);
					Vector3f new_force1 = (force1 == 0.0f) ? 0.0f : force1.reflect(direction0);
					float force_avg = (new_force0.length() + new_force1.length()) / 2;
					new_force0 = ((new_force0 == 0.0f) ? force1.normalise() : new_force0.normalise()) * force_avg;
					new_force1 = ((new_force1 == 0.0f) ? force0.normalise() : new_force1.normalise()) * force_avg;

					elem->setVeloctiy(new_momentum0 / elem->getMass());
					elem1->setVeloctiy(new_momentum1 / elem1->getMass());

					//elem->AddForce(new_force0 - force0);
					//elem1->AddForce(new_force1 - force1);
					elem->setAcceleration(0);
					elem1->setAcceleration(0);

					elem->Update(delta);
					elem1->Update(delta);
				}
			}
		}

		physicsComponents.clear();
	}

	void PhysicsEngine::addPhysicsComponent(PhysicsComponent* physicsComponent)
	{
		physicsComponents.push_back(physicsComponent);
	}
};
