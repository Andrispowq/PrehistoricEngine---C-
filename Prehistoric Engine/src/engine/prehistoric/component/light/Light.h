#ifndef LIGHT_H
#define LIGHT_H

#include "engine/prehistoric/component/Component.h"

class Light : public Component
{
public:
	Light(const Vector3f& colour = 0, const Vector3f& intensity = 1, bool sun = false);
	virtual ~Light() {}

	virtual void PreRender(Renderer* renderer) override;

	inline Vector3f getColour() const { return colour; }
	inline Vector3f getIntensity() const { return intensity; }

	inline bool isSun() const { return sun; }

	inline void setColour(const Vector3f& colour) { this->colour = colour; }
	inline void setIntensity(const Vector3f& intensity) { this->intensity = intensity; }

	inline void setSun(bool sun) { this->sun = sun; }
private:
	Vector3f colour;
	Vector3f intensity;

	bool sun;
};

#endif