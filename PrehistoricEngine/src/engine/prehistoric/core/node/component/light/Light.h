#ifndef LIGHT_H
#define LIGHT_H

#include "prehistoric/core/node/component/Component.h"

namespace Prehistoric
{
	class Light : public Component
	{
	public:
		Light(const Vector3f& colour = 0, float intensity = 1, float radius = 200.0f);
		virtual ~Light();

		virtual void PreRender(Renderer* renderer) override;

		inline Vector3f getColour() const { return colour; }
		inline float getIntensity() const { return intensity; }
		inline float getRadius() const { return radius; }

		inline void setColour(const Vector3f& colour) { this->colour = colour; }
		inline void setIntensity(float intensity) { this->intensity = intensity; }
		inline void setRadius(float radius) { this->radius = radius; }

		inline void SetToBeRegistered(bool set) { this->toBeRegistered = set; }

		static ComponentType getStaticComponentType() { return ComponentType::LightComponent; }

	private:
		Vector3f colour;
		float intensity;
		float radius;
		bool toBeRegistered;
	};
};

#endif