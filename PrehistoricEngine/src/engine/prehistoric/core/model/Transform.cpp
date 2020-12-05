#include "engine/prehistoric/core/util/Includes.hpp"
#include "Transform.h"

namespace Prehistoric
{
	void Transform::calculateTransformationMatrix()
	{
		transformationMatrix = Matrix4f::Transformation(position, rotation, scaling);
	}

	Matrix4f Transform::getModelViewProjectionMatrix(const Camera& camera) const
	{
		return camera.getViewProjectionMatrix() * getTransformationMatrix();
	}

	Matrix4f Transform::getTransformationMatrix(const Vector3f& position, const Vector3f& rotation, const Vector3f& scaling)
	{
		return Matrix4f::Transformation(position, rotation, scaling);
	}
};
