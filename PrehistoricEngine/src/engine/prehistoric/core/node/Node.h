#ifndef NODE_H
#define NODE_H

#include "Includes.hpp"

#include "prehistoric/core/model/Transform.h"
#include "prehistoric/core/util/guid/GUID.h"

namespace Prehistoric
{
    class CoreEngine;
    class Renderer;

    class Node
    {
    public:
        Node() : parent(nullptr), enabled(true) {}
        virtual ~Node() {}

        virtual void PreUpdate(CoreEngine* engine);
        virtual void PreRender(Renderer* renderer);

        inline const Transform& getWorldTransform() const { return worldTransform; }
        inline Transform& getWorldTransform() { return worldTransform; }
        inline Node* getParent() { return parent; }

        Node* getChild(const std::string& name) const;

        Node* AddChild(const std::string& name, Node* child);
        void RemoveChild(Node* node);
        void RemoveChild(const std::string& name);

        inline void Move(const Vector3f& d) { worldTransform.setPosition(worldTransform.getPosition() + d); }
        inline void Rotate(const Vector3f& d) { worldTransform.setRotation(worldTransform.getRotation() + d); }
        inline void Scale(const Vector3f& d) { worldTransform.setScaling(worldTransform.getScaling() + d); }

        inline void SetPosition(const Vector3f& d) { worldTransform.setPosition(d); }
        inline void SetRotation(const Vector3f& d) { worldTransform.setRotation(d); }
        inline void SetScale(const Vector3f& d) { worldTransform.setScaling(d); }

        inline bool isEnabled() const { return enabled; }
        inline void setEnabled(bool enabled) { this->enabled = enabled; }

        inline std::string getName() const { return name; }
        inline std::string& getName() { return name; }
        inline void setName(const std::string& name) { this->name = name; }

        std::unordered_map<GUID, Node*> getChildrenByID() const;
        std::unordered_map<std::string, Node*> getChildren() const;

        Node(const Node&) = default;
    protected:
        std::unordered_map<GUID, std::unique_ptr<Node>> children;
        Node* parent;

        std::string name;

        Transform worldTransform;
        bool enabled;
    };
};

#endif