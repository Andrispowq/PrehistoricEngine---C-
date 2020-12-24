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

		template<typename T> 
		T* GetComponent() const;

		GameObject(const GameObject&) = default;
	protected:
		std::unordered_map<std::string, std::unique_ptr<Component>> components;
		UpdateFunction updateFunction;
	};

	template<typename T>
	inline T* GameObject::GetComponent() const
	{
		for (auto component : components)
		{
			if (component.second->getComponentType() == T::getStaticComponentType())
			{
				return component.second.get();
			}
		}
	}
};

#endif