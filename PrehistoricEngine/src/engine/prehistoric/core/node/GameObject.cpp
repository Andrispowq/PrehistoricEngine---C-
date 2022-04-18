#include "Includes.hpp"
#include "GameObject.h"

#include "prehistoric/core/CoreEngine.h"

namespace Prehistoric
{
	void GameObject::PreUpdate(CoreEngine* engine)
	{
		if (!enabled)
			return;

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
		if (!enabled)
			return;

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

	std::vector<GameObject*> GameObject::getAllChildren() const
	{
		std::vector<GameObject*> _children;

		for (auto& child : children)
		{
			auto child_children = dynamic_cast<GameObject*>(child.second.get())->getAllChildren();
			for (auto c : child_children)
			{
				_children.push_back(c);
			}

			_children.push_back(dynamic_cast<GameObject*>(child.second.get()));
		}

		return _children;
	}

	std::unordered_map<std::string, Component*> GameObject::getComponents() const
	{
		std::unordered_map<std::string, Component*> map;
		map.reserve(components.size());

		for (const auto& elem : components)
		{
			map.insert(std::make_pair(elem.first, elem.second.get()));
		}

		return map;
	}
};
