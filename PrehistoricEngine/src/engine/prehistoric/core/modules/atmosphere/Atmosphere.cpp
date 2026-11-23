#include "Includes.hpp"
#include "Atmosphere.h"
#include "prehistoric/common/rendering/Renderer.h"

namespace Prehistoric
{
    void Atmosphere::PreRender(Renderer* renderer)
    {
        if (!isEnabled()) return;
        if (sun) renderer->RegisterAtmosphere(*this);
        GameObject::PreRender(renderer);
    }

    //We go around in a circle, from -range to range on the y and z axes
    void Atmosphere::sunMoveFunction(Prehistoric::GameObject *object, float frameTime) {
        constexpr float range = 32000.0f;
        constexpr float anglesPerSecond = 0.5f;

        static float angle = 130.0f;

        float x = cos(ToRadians(angle)) * range;
        float y = sin(ToRadians(angle)) * range;
        angle -= (anglesPerSecond * frameTime);

        object->SetPosition({x, y, 0});
    }
};