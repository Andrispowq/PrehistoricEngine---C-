#ifndef CAMERA_H
#define CAMERA_H

#include "prehistoric/common/framework/CameraInput.h"

#include "prehistoric/core/config/EngineConfig.h"

namespace Prehistoric
{
    class PR_API Camera
    {
    public:
        Camera(float moveAmt = 0.1f, float rotAmt = 0.8f, float mouseSensitivity = 0.8f, float fov = 70.0f, const Vector3f& position = Vector3f()); //There coordinates are just to spawn on a mountain
        Camera(const Vector3f& position, float fov);

        virtual ~Camera();

        void LogStage() const;
        void SetProjection(float fov, float width, float height);

        void AddCameraInput(CameraInput input) { inputs.push_back(input); }

        void Update(Window* window, float delta);
        void Move(const Vector3f& dir, const float& amount);
        void RotateX(const float& angle);
        void RotateY(const float& angle);

        Vector3f getLeft() const { return forward.cross(up).normalise(); }
        Vector3f getRight() const { return up.cross(forward).normalise(); }

        inline Vector3f getPosition() const { return position; }
        inline Vector3f getForward() const { return forward; }
        inline Vector3f getUp() const { return up; }

        inline float getFov() const { return fovY; }

        inline Matrix4f getViewMatrix() const { return viewMatrix; }
        inline Matrix4f getProjectionMatrix() const { return projectionMatrix; }
        inline Matrix4f getViewProjectionMatrix() const { return viewProjectionMatrix; }

        inline void setSpeedControl(InputData data) { this->speedControl = data; }

        bool isChanged() const { return cameraMoved || cameraRotated; }
    private:
        std::vector<CameraInput> inputs;
        InputData speedControl;

        Vector3f position;
        Vector3f previousPosition;
        Vector3f forward;
        Vector3f previousForward;
        Vector3f up;
        float movAmt;
        float rotAmt;
        Matrix4f viewMatrix;
        Matrix4f projectionMatrix;
        Matrix4f viewProjectionMatrix;
        Matrix4f previousViewMatrix;
        Matrix4f previousViewProjectionMatrix;
        bool cameraMoved;
        bool cameraRotated;

        float width;
        float height;
        float fovY;

        float rotYstride;
        float rotYamt;
        float rotYcounter;
        bool rotYInitiated = false;
        float rotXstride;
        float rotXamt;
        float rotXcounter;
        bool rotXInitiated = false;
        float mouseSensitivity;

        Vector4f* frustumPlanes;
        Vector3f* frustumCorners;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif