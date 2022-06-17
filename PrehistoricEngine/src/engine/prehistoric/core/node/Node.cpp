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
        child->setName(key);

        GUID guid = GenerateGUID(key);

        children.insert(std::make_pair(guid, child));
        return child;
    }

    Node* Node::getChild(const std::string& key) const
    {
        for (auto& entry : children)
        {
            if (entry.second->name == key)
            {
                return entry.second.get();
            }
        }
        return nullptr;
    }

    void Node::RemoveChild(Node* node)
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

    void Node::RemoveChild(const std::string& name)
    {
        for (auto& entry : children)
        {
            if (entry.second->name == name)
            {
                children.erase(entry.first);
                return;
            }
        }
    }

    std::unordered_map<GUID, Node*> Node::getChildrenByID() const
    {
        std::unordered_map<GUID, Node*> map;
        map.reserve(children.size());

        for (const auto& elem : children)
        {
            map.insert(std::make_pair(elem.first, elem.second.get()));
        }

        return map;

    }

    std::unordered_map<std::string, Node*> Node::getChildren() const
    {
        std::unordered_map<std::string, Node*> map;
        map.reserve(children.size());

        for (const auto& elem : children)
        {
            map.insert(std::make_pair(elem.second->name, elem.second.get()));
        }

        return map;
    }
};
