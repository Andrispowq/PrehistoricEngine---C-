#ifndef LIGHT_H
#define LIGHT_H

#include "prehistoric/core/node/component/Component.h"

namespace Prehistoric
{
	class Renderer;

	class Light : public Component
	{
	public:
		Light(const Vector3f& colour = 0, float intensity = 1, float radius = 200.0f, bool illuminate = true, bool castShadow = false);
		virtual ~Light();

		virtual void PreRender(Renderer* renderer) override;

		inline Vector3f getColour() const { return colour; }
		inline float getIntensity() const { return intensity; }
		inline float getRadius() const { return radius; }

		inline bool illuminates() const { return illuminate; }
		inline bool castShadows() const { return castShadow; }

		inline void setColour(const Vector3f& colour) { this->colour = colour; }
		inline void setIntensity(float intensity) { this->intensity = intensity; }
		inline void setRadius(float radius) { this->radius = radius; }

		inline void setIlluminate(bool set) { this->illuminate = set; }
		inline void setCastShadow(bool set) { this->castShadow = set; }

		static ComponentType getStaticComponentType() { return ComponentType::LightComponent; }

	private:
		Vector3f colour;
		float intensity;
		float radius;

		bool illuminate;
		bool castShadow;
	};
};

#endif