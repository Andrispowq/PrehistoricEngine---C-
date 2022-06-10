#ifndef SCRIPTABLE_CAMERA_H
#define SCRIPTABLE_CAMERA_H

#include "Camera.h"

namespace Prehistoric
{
    class ScriptableCamera : public Camera
    {
    public:
        ScriptableCamera(const Vector3f& position = Vector3f(), float fov = 70.0f, float moveAmt = 0.1f, float rotAmt = 0.8f, float mouseSensitivity = 0.8f) {}
        ~ScriptableCamera() {}

        virtual void UpdateCamera(Window* window, float delta) override;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif