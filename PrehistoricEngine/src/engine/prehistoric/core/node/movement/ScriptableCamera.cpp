#include "Includes.hpp"
#include "ScriptableCamera.h"

namespace Prehistoric
{
	void ScriptableCamera::UpdateCamera(Window* window, float delta)
	{
		this->previousViewMatrix = viewMatrix;
		viewMatrix = Matrix4f::View(forward, up) * Matrix4f::Translation(position * -1);
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}
}