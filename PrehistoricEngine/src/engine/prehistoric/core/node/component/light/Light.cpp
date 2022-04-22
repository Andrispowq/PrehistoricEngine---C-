#include "Includes.hpp"
#include "Light.h"

#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	Light::Light(const Vector3f& colour, float intensity, float radius, bool illuminate, bool castShadow)
		: colour(colour), intensity(intensity), radius(radius), illuminate(illuminate), castShadow(castShadow)
	{
		type = ComponentType::LightComponent;
	}

	Light::~Light()
	{
	}

	void Light::PreRender(Renderer* renderer)
	{
		bool shouldRegister = parent->isEnabled();

		if (shouldRegister)
		{
			renderer->RegisterLight(this);
		}
	}
};