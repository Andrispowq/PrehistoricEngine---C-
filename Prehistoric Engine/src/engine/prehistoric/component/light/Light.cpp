#include "engine/prehistoric/core/util/Includes.hpp"
#include "Light.h"

#include "engine/prehistoric/common/rendering/Renderer.h"

Light::Light(const Vector3f& colour, const Vector3f& intensity, bool sun)
	: colour(colour), intensity(intensity), sun(sun)
{
}

void Light::PreRender(Renderer* renderer)
{
	renderer->AddLight(this);
}