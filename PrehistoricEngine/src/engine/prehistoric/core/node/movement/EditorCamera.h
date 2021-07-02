#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include "Camera.h"

namespace Prehistoric
{
    class EditorCamera : public Camera
    {
    public:
        EditorCamera(const Vector3f& position = Vector3f(), float fov = 70.0f, float moveAmt = 0.1f, float rotAmt = 0.8f, float mouseSensitivity = 0.8f) {}
        ~EditorCamera() {}

        virtual void UpdateCamera(Window* window, float delta) override;

    private:
        Vector3f centre = 0;
        float baseRotation = 0;

        float distanceFromCentre = 10.0f;
        float angleAroundCentre = 180.0f;
        float pitch = 10.0f, yaw = 0, roll = 0;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif