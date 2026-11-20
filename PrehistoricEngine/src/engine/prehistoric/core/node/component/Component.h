#ifndef COMPONENT_H
#define COMPONENT_H

#include "prehistoric/core/node/Node.h"

namespace Prehistoric
{
	class GameObject;

	enum class ComponentType
	{
		General,
		AudioComponent,
		RenderableComponent,
		RendererComponent,
		LightComponent,
		PhysicsComponent,
		ScriptComponent,
		CameraComponent
	};

	class Component
	{
	public:
		Component() : parent(nullptr), type(ComponentType::General) {}
		virtual ~Component() {}

		virtual void OnInit() {}

		virtual void PreUpdate(CoreEngine* engine) {}
		virtual void PreRender(Renderer* renderer) {}

		inline GameObject* getParent() const { return parent; }
		inline void setParent(GameObject* parent) { this->parent = parent; }

		virtual ComponentType getComponentType() const { return type; };
		static ComponentType getStaticComponentType() { return ComponentType::ScriptComponent; };
	protected:
		GameObject* parent;
		ComponentType type;
	};
};

#endif