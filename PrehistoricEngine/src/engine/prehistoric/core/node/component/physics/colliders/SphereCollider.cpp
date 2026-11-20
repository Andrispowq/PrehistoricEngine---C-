#include "Includes.hpp"
#include "SphereCollider.h"

namespace Prehistoric
{
    SphereCollider::SphereCollider()
    {
        colliderType = ColliderType::SPHERE;
    }

    void SphereCollider::Update(GameObject* parent)
    {
        center = parent->getWorldTransform().getPosition();
        radius = parent->getWorldTransform().getScaling().x;
    }

    bool SphereCollider::Collide(Collider* other, HitInfo& info) const
    {
        if (other == nullptr)
        {
            return false;
        }

        if (other->getColliderType() == SphereCollider::GetType())
        {
            SphereCollider* otherS = (SphereCollider*)other;

            Vector3f distanceOfCenters = center - otherS->center;
            float dist = distanceOfCenters.length();

            bool hit = dist < (radius + otherS->radius);
            info.direction = distanceOfCenters;
            return hit;
        }
        else
        {
            return false;
        }
    }
};
