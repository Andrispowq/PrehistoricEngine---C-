#ifndef COLLIDER_H
#define COLLIDER_H

#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	struct HitInfo
	{
		Vector3f direction;
	};

	enum class ColliderType
	{
		NONE = 0x0,
		SPHERE = 0x1,
		TERRAIN = 0x2,
	};

	class Collider
	{
	public:
		Collider() {}
		virtual ~Collider();

		virtual void Update(GameObject* parent) = 0;

		//This function must return true if a collision happened, and false if a collision didn't happen, or the two types of colliders provided don't have a Collide function
		virtual bool Collide(Collider* other, HitInfo& info) const = 0;

		ColliderType getColliderType() const { return colliderType; }

		static ColliderType GetType() { return ColliderType::NONE; }
	protected:
		ColliderType colliderType = ColliderType::NONE;
	};
}

#endif