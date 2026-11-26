#include "Includes.hpp"
#include "GameObject.h"
#include "prehistoric/core/scene/world/ProceduralWorld.h"

#include "prehistoric/core/CoreEngine.h"
#include "prehistoric/core/scene/Scene.h"

namespace Prehistoric {
    void GameObject::CheckComponentRemoval(const Component *component) const {
        if (auto *scene = GetScene()) scene->CheckComponentRemoval(component);
    }

    std::unique_ptr<GameObject> GameObject::Clone() const {
        if (typeid(*this) != typeid(GameObject))
            throw std::runtime_error("This specialised object does not support duplication");
        auto copy = std::make_unique<GameObject>();
        CopyAuthoredStateTo(*copy);
        return copy;
    }

    void GameObject::CopyAuthoredStateTo(GameObject &copy) const {
        copy.worldTransform = worldTransform;
        copy.enabled = enabled;
        copy.updateFunction = updateFunction;
        for (const auto &[name, component]: components) {
            if (component->IsGenerated()) continue;
            auto cloned = component->Clone();
            copy.AddComponent(name, cloned.release());
        }
        for (const auto &[id, child]: children) {
            if (child->IsGenerated()) continue;
            auto *object = dynamic_cast<GameObject *>(child.get());
            if (!object) throw std::runtime_error("Cannot duplicate a non-GameObject child");
            auto cloned = object->Clone();
            copy.AddChild(child->getName(), cloned.release());
        }
        RemapProceduralReferences(*this, copy);
    }

    void GameObject::PreUpdate(CoreEngine *engine) {
        if (!enabled)
            return;

        for (auto &kv: components) {
            kv.second->PreUpdate(engine);
        }

        Node::PreUpdate(engine);

        if (updateFunction)
            updateFunction(this, engine->getFrameTime());
    }

    void GameObject::PreRender(Renderer *renderer) {
        if (!enabled)
            return;

        for (auto &kv: components) {
            kv.second->PreRender(renderer);
        }

        Node::PreRender(renderer);
    }

    GameObject *GameObject::AddComponent(const std::string &name, Component *component) {
        component->setParent(this);
        components.emplace(name, component);

        component->OnInit();
        return this;
    }

    Component *GameObject::GetComponent(const std::string &name) const {
        return components.at(name).get();
    }

    std::vector<GameObject *> GameObject::getAllChildren() const {
        std::vector<GameObject *> _children;

        for (auto &child: children) {
            auto *object = dynamic_cast<GameObject *>(child.second.get());
            if (!object) continue;
            auto child_children = object->getAllChildren();
            for (auto c: child_children) {
                _children.push_back(c);
            }

            _children.push_back(object);
        }

        return _children;
    }

    std::unordered_map<std::string, Component *> GameObject::getComponents() const {
        std::unordered_map<std::string, Component *> map;
        map.reserve(components.size());

        for (const auto &elem: components) {
            map.insert(std::make_pair(elem.first, elem.second.get()));
        }

        return map;
    }
};