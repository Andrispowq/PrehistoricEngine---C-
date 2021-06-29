#include "Includes.hpp"
#include "Light.h"

#include "prehistoric/common/rendering/Renderer.h"
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric
{
	Light::Light(const Vector3f& colour, const Vector3f& intensity, float radius)
		: colour(colour), intensity(intensity), radius(radius), toBeRegistered(true)
	{
		type = ComponentType::LightComponent;
	}

	Light::~Light()
	{
	}

	void Light::PreRender(Renderer* renderer)
	{
		bool shouldRegister = parent->isEnabled();

		if (toBeRegistered)
		{
			renderer->RegisterLight(this);
		}
	}
};