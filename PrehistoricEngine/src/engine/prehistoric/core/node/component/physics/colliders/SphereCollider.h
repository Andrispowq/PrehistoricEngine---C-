#ifndef SPHERE_COLLIDER_H
#define SPHERE_COLLIDER_H

#include "prehistoric/core/node/component/physics/colliders/Collider.h"

namespace Prehistoric
{
	class SphereCollider : public Collider
	{
	public:
		SphereCollider();
		virtual ~SphereCollider() {}

		virtual void Update(GameObject* parent) override;

		virtual bool Collide(Collider* other, HitInfo& info) const override;

		static ColliderType GetType() { return ColliderType::SPHERE; }
	public:
		Vector3f center = 0;
		float radius = 0.0f;
	};
}

#endif