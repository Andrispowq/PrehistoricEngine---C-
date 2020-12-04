#include "engine/prehistoric/core/util/Includes.hpp"
#include "Camera.h"

static const Vector3f yAxis(0, 1, 0);

Camera::Camera(float moveAmt, float rotAmt, float mouseSensitivity, float fov, const Vector3f& position)
{
	this->movAmt = moveAmt;
	this->rotAmt = rotAmt;
	this->mouseSensitivity = mouseSensitivity;

	this->position = position;
	this->forward = Vector3f(0, 0, 1);
	this->up = yAxis;

	SetProjection(fov, (float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight);

	this->viewMatrix = Matrix4f::View(forward, up) * Matrix4f::Translation(position * -1);
	this->viewProjectionMatrix = Matrix4f::Zero();

	previousViewMatrix = Matrix4f::Zero();
	previousViewProjectionMatrix = Matrix4f::Zero();

	frustumPlanes = new Vector4f[6];
	frustumCorners = new Vector3f[8];
}

Camera::Camera(const Vector3f& position, float fov)
{
	this->movAmt = 0.1f;
	this->rotAmt = 0.8f;
	this->mouseSensitivity = 0.8f;

	this->position = position;
	this->forward = Vector3f(0, 0, 1);
	this->up = yAxis;

	SetProjection(fov, (float)FrameworkConfig::windowWidth, (float)FrameworkConfig::windowHeight);

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

	movAmt += CameraInput::GetKey(speedControl) * delta * 35.0f;
	movAmt = std::fmax(0.02f, movAmt);

	//Move by input systems
	bool movedForward = false, movedBackward = false, movedRight = false, movedLeft = false,
		rotUp = false, rotDown = false, rotRight = false, rotLeft = false;

	for (CameraInput& in : inputs)
	{
		if (in.isForward() != 0 && !movedForward)
		{
			Move(forward, movAmt * in.isForward() * delta);
			movedForward = true;
		}
		if (in.isBackward() != 0 && !movedBackward)
		{
			Move(forward, -movAmt * in.isBackward() * delta);
			movedBackward = true;
		}
		if (in.isLeft() != 0 && !movedLeft)
		{
			Move(getLeft(), movAmt * in.isLeft() * delta);
			movedLeft = true;
		}
		if (in.isRight() != 0 && !movedRight)
		{
			Move(getLeft(), -movAmt * in.isRight() * delta);
			movedRight = true;
		}

		if (in.isUp() != 0 && !rotUp)
		{
			RotateX(static_cast<float>(-rotAmt * 2.0 * in.isUp() * delta));
			rotUp = true;
		}
		if (in.isDown() != 0 && !rotDown)
		{
			RotateX(static_cast<float>(rotAmt * 2.0 * in.isDown() * delta));
			rotDown = true;
		}
		if (in.isRightRot() != 0 && !rotRight)
		{
			RotateY(static_cast<float>(rotAmt * 2.0 * in.isRightRot() * delta));
			rotRight = true;
		}
		if (in.isLeftRot() != 0 && !rotLeft)
		{
			RotateY(static_cast<float>(-rotAmt * 2.0 * in.isLeftRot() * delta));
			rotLeft = true;
		}
	}

	//Free mouse rotation
	if (InputInstance.IsButtonHeld((InputCode) 2))
	{
		float dy = InputInstance.getLockedCursorPosition().y - InputInstance.getCursorPosition().y;
		float dx = InputInstance.getLockedCursorPosition().x - InputInstance.getCursorPosition().x;

		//y-axis rotation
		if (dy != 0)
		{
			rotYstride = std::fabs(dy * 0.01f);
			rotYamt = -dy;
			rotYcounter = 0;
			rotYInitiated = true;
		}

		if (rotYInitiated)
		{
			//up-rotation
			if (rotYamt < 0)
			{
				if (rotYcounter > rotYamt)
				{
					RotateX(-rotYstride * mouseSensitivity);
					rotYcounter -= rotYstride;
					rotYstride *= 0.98f;
				}
				else
				{
					rotYInitiated = false;
				}
			}
			//down-rotation
			else if (rotYamt > 0)
			{
				if (rotYcounter < rotYamt)
				{
					RotateX(rotYstride * mouseSensitivity);
					rotYcounter += rotYstride;
					rotYstride *= 0.98f;
				}
				else
				{
					rotYInitiated = false;
				}
			}
		}

		//x-axis rotation
		if (dx != 0)
		{
			rotXstride = std::fabs(dx * 0.01f);
			rotXamt = -dx;
			rotXcounter = 0;
			rotXInitiated = true;
		}

		if (rotXInitiated)
		{
			//up-rotation
			if (rotXamt < 0)
			{
				if (rotXcounter > rotXamt)
				{
					RotateY(-rotXstride * mouseSensitivity);
					rotXcounter -= rotXstride;
					rotXstride *= 0.98f;
				}
				else
				{
					rotXInitiated = false;
				}
			}
			//down-rotation
			else if (rotXamt > 0)
			{
				if (rotXcounter < rotXamt)
				{
					RotateY(rotXstride * mouseSensitivity);
					rotXcounter += rotXstride;
					rotXstride *= 0.98f;
				}
				else
				{
					rotXInitiated = false;
				}
			}
		}

		InputInstance.setCursorPositionOnScreen(window, InputInstance.getLockedCursorPosition());
	}

	if (position != previousPosition)
	{
		cameraMoved = true;
	}
	if (forward != previousForward)
	{
		cameraRotated = true;
	}

	this->previousViewMatrix = viewMatrix;
	viewMatrix = Matrix4f::View(forward, up) * Matrix4f::Translation(position * -1);
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Camera::Move(const Vector3f& dir, const float& amount)
{
	this->position += dir * amount;
}

void Camera::RotateX(const float& angle)
{
	Vector3f hAxis = yAxis.cross(forward).normalise();
	this->forward = forward.rotate(hAxis, angle).normalise();
	this->up = forward.cross(hAxis).normalise();
}

void Camera::RotateY(const float& angle)
{
	Vector3f hAxis = yAxis.cross(forward).normalise();
	this->forward = forward.rotate(yAxis, angle).normalise();
	this->up = forward.cross(hAxis).normalise();
}

void Camera::SetProjection(float fov, float width, float height)
{
	this->fovY = fov;
	this->width = width;
	this->height = height;

	this->projectionMatrix = Matrix4f::PerspectiveProjection(fovY, width / height, EngineConfig::rendererNearPlane, EngineConfig::rendererFarPlane);
	if (FrameworkConfig::api == Vulkan)
	{
		projectionMatrix.m[1 * 4 + 1] *= -1;
	}
}