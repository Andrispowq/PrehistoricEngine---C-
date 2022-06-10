using System;

public class CameraController : BaseComponent
{
    public static float SPEED = 5.0f;

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);
    }

    public override void OnUpdate(float delta)
    {
        if(HasComponent("CameraComponent"))
        {
            Camera camera = Camera.Get();

            camera.position.x -= GetJoystickAxis(0, 0) * SPEED * delta;
            camera.position.z += GetJoystickAxis(0, 1) * SPEED * delta;
            camera.forward = new Vector3f(0, 0, 1);

            SPEED -= GetJoystickAxis(0, 4) * 5.0f * delta;
            SPEED += GetJoystickAxis(0, 5) * 5.0f * delta;

            if(SPEED < 0.0f)
            {
                SPEED = 0.0f;
            }

            Camera.Set(camera);
        }
    }
}