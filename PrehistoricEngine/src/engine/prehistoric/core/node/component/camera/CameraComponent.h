#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "prehistoric/core/node/component/Component.h"

#include "prehistoric/core/node/movement/Camera.h"

namespace Prehistoric
{
	class CoreEngine;

	class CameraComponent : public Component
	{
	public:
		CameraComponent(bool primary = false);
		~CameraComponent();

		void PreUpdate(CoreEngine* engine) override;

		Camera* getCamera() { return camera; }

		bool IsPrimary() const { return primary; }
		void SetPrimary(bool primary) { this->primary = primary; }

		static ComponentType getStaticComponentType() { return ComponentType::CameraComponent; }

	private:
		Camera* camera = nullptr;
		bool primary = false;
	};
};

#endif