#ifndef LAYER_H
#define LAYER_H

#include "Includes.hpp"

#include "prehistoric/core/events/Event.h"

namespace Prehistoric
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}

		virtual void OnEvent(Event& event) {}

		virtual void Update(float delta) {}
		virtual void ImGUIRender() {}
		virtual void Render() {}

		const std::string& getName() const { return debugName; }

	protected:
		std::string debugName;
	};
};

#endif