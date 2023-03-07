#ifndef TERRAIN_COLLIDER_H
#define TERRAIN_COLLIDER_H

#include "prehistoric/core/node/component/physics/colliders/Collider.h"

#include "prehistoric/core/modules/terrain/Terrain.h"

namespace Prehistoric
{
	class TerrainCollider : public Collider
	{
	public:
		TerrainCollider();
		virtual ~TerrainCollider() {}

		virtual void Update(GameObject* parent) override;

		virtual bool Collide(Collider* other, HitInfo& info) const override;

		static ColliderType GetType() { return ColliderType::TERRAIN; }
	public:
		Terrain* terrain;
	};
};

#endif