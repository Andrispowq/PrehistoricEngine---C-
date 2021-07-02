#include "Includes.hpp"
#include "EditorCamera.h"

#include "imgui.h"

namespace Prehistoric
{
	void EditorCamera::UpdateCamera(Window* window, float delta)
	{
		float zoom = InputInstance.getScrollOffset() * mouseSensitivity;
		distanceFromCentre -= zoom;

		if (InputInstance.IsButtonHeld((InputCode)1))
		{
			float dy = InputInstance.getCursorDeltaPosition().y;
			pitch += dy * 0.1f;
		}

		if (InputInstance.IsButtonHeld((InputCode)0))
		{
			float dx = InputInstance.getCursorDeltaPosition().x;
			angleAroundCentre -= dx * 0.3f;
		}

		float horizDist = distanceFromCentre * std::cos(ToRadians(pitch));
		float vertDist = distanceFromCentre * std::sin(ToRadians(pitch));

		float theta = baseRotation + angleAroundCentre;
		float offX = horizDist * std::sin(ToRadians(theta));
		float offZ = horizDist * std::cos(ToRadians(theta));
		position.x = centre.x - offX;
		position.z = centre.z - offZ;
		position.y = centre.y + vertDist;

		yaw = 180 - (baseRotation + theta);

		Matrix4f rot = Matrix4f::Rotation({ pitch, yaw, roll });
		Matrix4f translation = Matrix4f::Translation(position * -1);
		Matrix4f view = rot * translation;

		this->previousViewMatrix = viewMatrix;
		viewMatrix = view;
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}
};