#include "Includes.hpp"
#include "CameraComponent.h"

#include "prehistoric/core/node/movement/ScriptableCamera.h"

#include "prehistoric/core/CoreEngine.h"

namespace Prehistoric
{
	CameraComponent::CameraComponent(bool primary)
		: primary(primary)
	{
		camera = new ScriptableCamera();
		camera->Update(nullptr, 0.0f);
	}

	CameraComponent::~CameraComponent()
	{
		delete camera;
	}

	void CameraComponent::PreUpdate(CoreEngine* engine)
	{
		parent->SetPosition(camera->getPosition());
		engine->getRenderingEngine()->AddCameraComponent(this);
	}
};
