#ifndef SCRIPTABLE_CAMERA_H
#define SCRIPTABLE_CAMERA_H

#include "Camera.h"

namespace Prehistoric {
    class ScriptableCamera : public Camera {
    public:
        using Camera::Camera;

        ~ScriptableCamera() {
        }

        virtual void UpdateCamera(Window *window, float delta) override;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif