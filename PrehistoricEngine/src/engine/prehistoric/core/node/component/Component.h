#ifndef COMPONENT_H
#define COMPONENT_H

#include "prehistoric/core/node/Node.h"

namespace Prehistoric {
    class GameObject;

    enum class ComponentType {
        General,
        AudioComponent,
        RenderableComponent,
        RendererComponent,
        LightComponent,
        PhysicsComponent,
        ScriptComponent,
        CameraComponent,
        VolumetricSphereComponent
    };

    class Component {
    public:
        bool generated = false;

        Component() : parent(nullptr), type(ComponentType::General) {
        }

        bool IsGenerated() const { return generated; }
        void SetGenerated(bool value) { generated = value; }

        virtual ~Component() {
        }

        virtual std::unique_ptr<Component> Clone() const {
            throw std::runtime_error("This component does not support duplication");
        }

        virtual void OnInit() {
        }

        virtual void PreUpdate(CoreEngine *engine) {
        }

        virtual void PreRender(Renderer *renderer) {
        }

        inline GameObject *getParent() const { return parent; }
        inline void setParent(GameObject *parent) { this->parent = parent; }

        virtual ComponentType getComponentType() const { return type; };
        static ComponentType getStaticComponentType() { return ComponentType::ScriptComponent; };

    protected:
        GameObject *parent;
        ComponentType type;
    };
};

#endif