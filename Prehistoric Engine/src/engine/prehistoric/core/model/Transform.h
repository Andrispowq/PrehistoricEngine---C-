#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "engine/prehistoric/core/movement/Camera.h"

class Transform
{
public:
	Transform(const Vector3f& position, const Vector3f& rotation, const Vector3f& scaling)
		: position(position), rotation(rotation), scaling(scaling) { calculateTransformationMatrix(); }
	Transform() : position(Vector3f()), rotation(Vector3f()), scaling(Vector3f(1)) { calculateTransformationMatrix(); }

	virtual ~Transform() {}

	void calculateTransformationMatrix();

	static Matrix4f getTransformationMatrix(const Vector3f& position, const Vector3f& rotation = Vector3f(), const Vector3f& scaling = Vector3f(1));

	inline Vector3f getPosition() const { return position; }
	inline Vector3f getRotation() const { return rotation; }
	inline Vector3f getScaling() const { return scaling; }

	inline Matrix4f getTransformationMatrix() const { return transformationMatrix; }
	Matrix4f getModelViewProjectionMatrix(const Camera& camera) const;

	inline void setPosition(const Vector3f& position) { this->position = position; calculateTransformationMatrix(); }
	inline void setRotation(const Vector3f& rotation) { this->rotation = rotation; calculateTransformationMatrix(); }
	inline void setScaling(const Vector3f& scaling) { this->scaling = scaling; calculateTransformationMatrix(); }
private:
	Vector3f position;
	Vector3f rotation;
	Vector3f scaling;

	Matrix4f transformationMatrix;
};

#endif