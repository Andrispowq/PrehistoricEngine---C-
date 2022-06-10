#ifndef CAMERA_H
#define CAMERA_H

#include "prehistoric/common/framework/CameraInput.h"

namespace Prehistoric
{
    class Camera
    {
    public:
        Camera(const Vector3f& position = Vector3f(), float fov = 70.0f, float moveAmt = 0.1f, float rotAmt = 0.8f, float mouseSensitivity = 0.8f); //There coordinates are just to spawn on a mountain
        virtual ~Camera();

        void Update(Window* window, float delta);

        virtual void UpdateCamera(Window* window, float delta) = 0;

        void LogStage() const;
        void SetProjection(float fov, float width, float height);

        void AddCameraInput(CameraInput input) { inputs.push_back(input); }

        Vector3f getLeft() const { return forward.cross(up).normalise(); }
        Vector3f getRight() const { return up.cross(forward).normalise(); }

        inline Vector3f getPosition() const { return position; }
        inline Vector3f getForward() const { return forward; }
        inline Vector3f getUp() const { return up; }

        inline void setPosition(Vector3f position) { this->position = position; }
        inline void setForward(Vector3f forward) { this->forward = forward; }
        inline void setUp(Vector3f up) { this->up = up; }

        inline float getFov() const { return fovY; }

        inline const Matrix4f& getViewMatrix() const { return viewMatrix; }
        inline const Matrix4f& getProjectionMatrix() const { return projectionMatrix; }
        inline const Matrix4f& getViewProjectionMatrix() const { return viewProjectionMatrix; }

        bool isChanged() const { return cameraMoved || cameraRotated; }

    protected:
        std::vector<CameraInput> inputs = {};

        Vector3f position = 0;
        Vector3f previousPosition = 0;
        Vector3f forward = 0;
        Vector3f previousForward = 0;
        Vector3f up = 0;

        float movAmt = 0.0f;
        float rotAmt = 0.0f;
        float mouseSensitivity = 0.0f;

        Matrix4f viewMatrix;
        Matrix4f projectionMatrix;
        Matrix4f viewProjectionMatrix;
        Matrix4f previousViewMatrix;
        Matrix4f previousViewProjectionMatrix;

        bool cameraMoved = false;
        bool cameraRotated = false;

        float width = 0.0f;
        float height = 0.0f;
        float fovY = 0.0f;

        Vector4f* frustumPlanes = nullptr;
        Vector3f* frustumCorners = nullptr;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif