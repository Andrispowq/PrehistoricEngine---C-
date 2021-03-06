#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Includes.hpp"

#include "prehistoric/core/node/component/Component.h"

namespace Prehistoric
{
	typedef void(*UpdateFunction)(GameObject*, float); //The object, and the delta time

	class GameObject : public Node
	{
	public:
		GameObject() : updateFunction(nullptr) {}
		virtual ~GameObject() {}

		void PreUpdate(CoreEngine* engine) override;
		void PreRender(Renderer* renderer) override;

		void setUpdateFunction(UpdateFunction func) { this->updateFunction = func; }

		GameObject* AddComponent(const std::string& name, Component* component);
		Component* GetComponent(const std::string& name) const;

		std::vector<GameObject*> getAllChildren() const;

		inline std::string getName() const { return name; }
		inline std::string& getName() { return name; }
		inline void setName(const std::string& name) { this->name = name; }

		template<typename T> 
		T* GetComponent() const;

		template<typename T>
		bool HasComponent() const;

		template<typename T>
		void RemoveComponent();

		GameObject(const GameObject&) = default;
	protected:
		std::unordered_map<std::string, std::unique_ptr<Component>> components;
		UpdateFunction updateFunction;
		std::string name;
	};

	template<typename T>
	inline T* GameObject::GetComponent() const
	{
		for (auto& component : components)
		{
			if (component.second->getComponentType() == T::getStaticComponentType())
			{
				return (T*)component.second.get();
			}
		}
		
		return nullptr;
	}

	template<typename T>
	inline bool GameObject::HasComponent() const
	{
		for (auto& component : components)
		{
			if (component.second->getComponentType() == T::getStaticComponentType())
			{
				return true;
			}
		}

		return false;
	}

	template<typename T>
	inline void GameObject::RemoveComponent()
	{
		std::vector<std::string> keys;

		for (auto& component : components)
		{
			if (component.second->getComponentType() == T::getStaticComponentType())
			{
				keys.push_back(component.first);
			}
		}

		for (auto& elem : keys)
		{
			components.erase(elem);
		}
	}
};

#endif