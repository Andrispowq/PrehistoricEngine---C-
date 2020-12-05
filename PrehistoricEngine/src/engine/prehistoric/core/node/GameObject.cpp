#include "engine/prehistoric/core/util/Includes.hpp"
#include "GameObject.h"

#include "engine/prehistoric/core/Engine.h"

namespace Prehistoric
{
	void GameObject::PreUpdate(Engine* engine)
	{
		for (auto& kv : components)
		{
			kv.second->PreUpdate(engine);
		}

		Node::PreUpdate(engine);

		if (updateFunction)
			updateFunction(this, engine->getFrameTime());
	}

	void GameObject::PreRender(Renderer* renderer)
	{
		for (auto& kv : components)
		{
			kv.second->PreRender(renderer);
		}

		Node::PreRender(renderer);
	}

	GameObject* GameObject::AddComponent(const std::string& name, Component* component)
	{
		component->setParent(this);
		components.emplace(name, component);
		return this;
	}

	Component* GameObject::GetComponent(const std::string& name) const
	{
		return components.at(name).get();
	}
};
