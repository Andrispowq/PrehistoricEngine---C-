#include "engine/prehistoric/core/util/Includes.hpp"
#include "Node.h"

#include "engine/prehistoric/core/Engine.h"

namespace Prehistoric
{
    void Node::PreUpdate(Engine* engine)
    {
        for (auto& child : children)
        {
            child.second->PreUpdate(engine);
        }
    }

    void Node::PreRender(Renderer* renderer)
    {
        for (auto& child : children)
        {
            child.second->PreRender(renderer);
        }
    }

    Node* Node::AddChild(const std::string& key, Node* child)
    {
        child->parent = this;
        children.insert(std::make_pair(key, child));
        return this;
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
