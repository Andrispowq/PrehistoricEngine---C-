#ifndef COMPONENT_H
#define COMPONENT_H

#include "prehistoric/core/node/Node.h"

namespace Prehistoric
{
	class GameObject;

	class Component
	{
	public:
		Component() : parent(nullptr) {}
		virtual ~Component() {}

		virtual void PreUpdate(CoreEngine* engine) {}
		virtual void PreRender(Renderer* renderer) {}

		inline GameObject* getParent() const { return parent; }
		inline void setParent(GameObject* parent) { this->parent = parent; }
	protected:
		GameObject* parent;
	};
};

#endif