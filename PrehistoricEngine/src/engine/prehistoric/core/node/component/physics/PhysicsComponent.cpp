#include "Includes.hpp"
#include "PhysicsComponent.h"
#include "colliders/SphereCollider.h"

#include "prehistoric/core/CoreEngine.h"

namespace Prehistoric {
    std::unique_ptr<Component> PhysicsComponent::Clone() const {
        auto copy = std::make_unique<PhysicsComponent>(mass, affectedByGravity);
        copy->velocity = velocity;
        copy->acceleration = acceleration;
        if (collider) {
            auto *sphere = dynamic_cast<SphereCollider *>(collider);
            if (!sphere) throw std::runtime_error("Only sphere collider duplication is supported");
            copy->clonedCollider = std::make_unique<SphereCollider>(*sphere);
            copy->collider = copy->clonedCollider.get();
        }
        return copy;
    }

    PhysicsComponent::PhysicsComponent(float mass, bool affectedByGravity)
        : mass(mass), affectedByGravity(affectedByGravity) {
    }

    PhysicsComponent::~PhysicsComponent() {
    }

    void PhysicsComponent::PreUpdate(CoreEngine *engine) {
        engine->getPhysicsEngine()->addPhysicsComponent(this);
    }

    void PhysicsComponent::Update(float delta) {
        if (affectedByGravity) {
            AddForce(gravity * mass * delta);
        }

        velocity += acceleration * delta;
        parent->Move(velocity * delta);

        if (collider) {
            collider->Update(parent);
        }
    }

    void PhysicsComponent::AddForce(Vector3f force) {
        acceleration += (force / mass);
    }
};