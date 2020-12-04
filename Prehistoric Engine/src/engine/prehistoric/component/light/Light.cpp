#include "engine/prehistoric/core/util/Includes.hpp"
#include "Light.h"

#include "engine/prehistoric/core/Engine.h"

Light::Light(const Vector3f& colour, const Vector3f& intensity, bool sun)
	: colour(colour), intensity(intensity), sun(sun)
{
}

void Light::PreUpdate(Engine* engine)
{
}

void Light::PreRender(Renderer* renderer)
{
	renderer->AddLight(this);
}