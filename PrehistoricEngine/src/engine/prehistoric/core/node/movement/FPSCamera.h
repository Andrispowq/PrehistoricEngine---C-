#ifndef FPS_CAMERA_H
#define FPS_CAMERA_H

#include "Camera.h"

namespace Prehistoric
{
    class FPSCamera : public Camera
    {
    public:
        FPSCamera(const Vector3f& position = Vector3f(), float fov = 70.0f, float moveAmt = 0.1f, float rotAmt = 0.8f, float mouseSensitivity = 0.8f) {}
        ~FPSCamera() {}

        virtual void UpdateCamera(Window* window, float delta) override;

        void Move(const Vector3f& dir, const float& amount);
        void RotateX(const float& angle);
        void RotateY(const float& angle);

    private:
        float rotYstride;
        float rotYamt;
        float rotYcounter;
        bool rotYInitiated = false;

        float rotXstride;
        float rotXamt;
        float rotXcounter;
        bool rotXInitiated = false;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif