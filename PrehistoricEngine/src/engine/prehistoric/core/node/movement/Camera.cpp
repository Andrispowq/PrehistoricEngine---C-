#include "Includes.hpp"
#include "Camera.h"

#include "imgui.h"

#include "prehistoric/application/Application.h"

namespace Prehistoric
{
	static const Vector3f yAxis(0, 1, 0);

	Camera::Camera(const Vector3f& position, float fov, float moveAmt, float rotAmt, float mouseSensitivity)
		: position(position), forward({ 0, 0, 1 }), up(yAxis),
			movAmt(movAmt), rotAmt(rotAmt), mouseSensitivity(mouseSensitivity)
	{
		SetProjection(fov, (float)__FrameworkConfig.windowWidth, (float)__FrameworkConfig.windowHeight);

		this->viewMatrix = Matrix4f::View(forward, up) * Matrix4f::Translation(position * -1);
		this->viewProjectionMatrix = Matrix4f::Zero();

		previousViewMatrix = Matrix4f::Zero();
		previousViewProjectionMatrix = Matrix4f::Zero();

		frustumPlanes = new Vector4f[6];
		frustumCorners = new Vector3f[8];
	}

	Camera::~Camera()
	{
		delete[] frustumPlanes;
		delete[] frustumCorners;
	}

	void Camera::LogStage() const
	{
		PR_LOG_MESSAGE("Camera information:\n");

		PR_LOG_MESSAGE("\tPosition: ");
		position.print();
		PR_LOG_MESSAGE("\tForward: ");
		forward.print();
		PR_LOG_MESSAGE("\tUp direction: ");
		up.print();
		PR_LOG_MESSAGE("\tRight direction: ");
		getRight().print();
		PR_LOG_MESSAGE("\tLeft direction: ");
		getLeft().print();
	}	

	void Camera::Update(Window* window, float delta)
	{
		this->previousPosition = position;
		this->previousForward = forward;

		cameraMoved = false;
		cameraRotated = false;

		UpdateCamera(window, delta);

		if (position != previousPosition)
		{
			cameraMoved = true;
		}
		if (forward != previousForward)
		{
			cameraRotated = true;
		}
	}

	void Camera::SetProjection(float fov, float width, float height)
	{
		this->fovY = fov;
		this->width = width;
		this->height = height;

		this->projectionMatrix = Matrix4f::PerspectiveProjection(fovY, width / height, __EngineConfig.rendererNearPlane, __EngineConfig.rendererFarPlane);
		if (__FrameworkConfig.api == Vulkan)
		{
			projectionMatrix.m[1 * 4 + 1] *= -1;
		}
	}
};
