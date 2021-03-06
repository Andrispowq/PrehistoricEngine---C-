#ifndef CAMERA_H
#define CAMERA_H

#include "prehistoric/common/framework/CameraInput.h"

#include "prehistoric/core/config/EngineConfig.h"

namespace Prehistoric
{
    enum class CameraControlType
    {
        FPS = 0,
        Editor = 1
    };

    class Camera
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

        inline const Matrix4f& getViewMatrix() const { return viewMatrix; }
        inline const Matrix4f& getProjectionMatrix() const { return projectionMatrix; }
        inline const Matrix4f& getViewProjectionMatrix() const { return viewProjectionMatrix; }

        inline void setSpeedControl(InputData data) { this->speedControl = data; }
        inline void setPosition(Vector3f pos) { this->position = pos; }

        inline void setCameraType(CameraControlType operationMode) { this->operationMode = operationMode; SynchroniseCameras(); }

        bool isChanged() const { return cameraMoved || cameraRotated; }
    private:

        void UpdateFPSCamera(Window* window, float delta);
        void UpdateEditorCamera(Window* window, float delta);
        void SynchroniseCameras(); //Makes it so that when you switch to FPS from Editor, you can continue from the same location and direction

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

        //For FPS camera
        float rotYstride;
        float rotYamt;
        float rotYcounter;
        bool rotYInitiated = false;
        float rotXstride;
        float rotXamt;
        float rotXcounter;
        bool rotXInitiated = false;
        float mouseSensitivity;

        //For editor camera
        float distanceFromCentre = 10.0f;
        float angleAroundWorld = 180.0f;
        float pitch = 10.0f, yaw = 0, roll = 0;

        CameraControlType operationMode = CameraControlType::Editor;

        Vector4f* frustumPlanes;
        Vector3f* frustumCorners;
    };
};

#include "prehistoric/core/util/math/MathT.h"

#endif