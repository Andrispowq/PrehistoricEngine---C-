#include "Includes.hpp"
#include "Light.h"

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	Light::Light(const Vector3f& colour, const Vector3f& intensity)
		: colour(colour), intensity(intensity), toBeRegistered(true)
	{
		type = ComponentType::LightComponent;
	}

	void Light::PreRender(Renderer* renderer)
	{
		if(toBeRegistered)
			renderer->AddLight(this);
	}
};