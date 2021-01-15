#include "Includes.hpp"
#include "Node.h"

#include "prehistoric/core/CoreEngine.h"
#include "GameObject.h"

namespace Prehistoric
{
    void Node::PreUpdate(CoreEngine* engine)
    {
        if (!enabled)
            return;

        for (auto& child : children)
        {
            child.second->PreUpdate(engine);
        }
    }

    void Node::PreRender(Renderer* renderer)
    {
        if (!enabled)
            return;

        for (auto& child : children)
        {
            child.second->PreRender(renderer);
        }
    }

    Node* Node::AddChild(const std::string& key, Node* child)
    {
        child->parent = this;

        GameObject* obj;
        if (obj = dynamic_cast<GameObject*>(child))
        {
            obj->setName(key);
        }

        children.insert(std::make_pair(key, child));
        return this;
    }

    void Node::deleteChild(Node* node)
    {
        for (auto& entry : children)
        {
            if (entry.second.get() == node)
            {
                children.erase(entry.first);
                return;
            }
        }
    }

    std::unordered_map<std::string, Node*> Node::getChildren() const
    {
        std::unordered_map<std::string, Node*> map;
        map.reserve(children.size());

        for (const auto& elem : children)
        {
            map.insert(std::make_pair(elem.first, elem.second.get()));
        }

        return map;
    }
};
