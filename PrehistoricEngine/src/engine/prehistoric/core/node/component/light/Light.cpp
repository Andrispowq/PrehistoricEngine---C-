#include "Includes.hpp"
#include "Light.h"

#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
	Light::Light(const Vector3f& colour, const Vector3f& intensity, bool sun)
		: colour(colour), intensity(intensity), sun(sun)
	{
	}

	void Light::PreRender(Renderer* renderer)
	{
		renderer->AddLight(this);
	}
};