#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "engine/prehistoric/core/util/Includes.hpp"

#include "engine/prehistoric/core/node/component/Component.h"

typedef void(*UpdateFunction)(GameObject*, float); //The object, and the delta time

class GameObject : public Node
{
public:
	GameObject() : updateFunction(nullptr) {}
	virtual ~GameObject() {}

	void PreUpdate(Engine* engine) override;
	void PreRender(Renderer* renderer) override;

	void setUpdateFunction(UpdateFunction func) { this->updateFunction = func; }

	GameObject* AddComponent(const std::string& name, Component* component);
	Component* GetComponent(const std::string& name) const;

	GameObject(const GameObject&) = default;
protected:
	std::unordered_map<std::string, std::unique_ptr<Component>> components;
	UpdateFunction updateFunction;
};

#endif