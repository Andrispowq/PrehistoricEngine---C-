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

        inline float getFov() const { return fovY; }

        inline const Matrix4f& getViewMatrix() const { return viewMatrix; }
        inline const Matrix4f& getProjectionMatrix() const { return projectionMatrix; }
        inline const Matrix4f& getViewProjectionMatrix() const { return viewProjectionMatrix; }

        bool isChanged() const { return cameraMoved || cameraRotated; }

        inline void setPosition(Vector3f pos) { this->position = pos; }

    protected:
        std::vector<CameraInput> inputs;

        Vector3f position;
        Vector3f previousPosition;
        Vector3f forward;
        Vector3f previousForward;
        Vector3f up;

        float movAmt;
        float rotAmt;
        float mouseSensitivity;

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

        Vector4f* frustumPlanes;
        Vector3f* frustumCorners;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif