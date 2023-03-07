#include "Includes.hpp"
#include "TerrainCollider.h"

#include "SphereCollider.h"

namespace Prehistoric
{
	TerrainCollider::TerrainCollider()
	{
		colliderType = ColliderType::TERRAIN;
	}

    void TerrainCollider::Update(GameObject* parent)
    {
    }

    bool TerrainCollider::Collide(Collider* other, HitInfo& info) const
    {
        if (other == nullptr)
        {
            return false;
        }

        if (other->getColliderType() == SphereCollider::GetType())
        {
            SphereCollider* otherS = (SphereCollider*)other;

            int terrainX = (int)(terrain->getWorldTransform().getPosition().x - otherS->center.x);
            int terrainZ = (int)(terrain->getWorldTransform().getPosition().z - otherS->center.z);


        }
        else
        {
            return false;
        }
    }
};
