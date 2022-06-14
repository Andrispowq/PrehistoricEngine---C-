using System;

public class CameraController : BaseComponent
{
    public static float SPEED = 5.0f;
    public static float ROTATE_SPEED = 180.0f;

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);
    }

    public override void OnUpdate(float delta)
    {
        if(HasComponent("CameraComponent"))
        {
            Camera camera = Camera.Get();
            //camera.forward = new Vector3f(0, 0, 1);

            camera.Move(camera.GetLeft(), GetJoystickAxis(0, 0) * SPEED * delta);
            camera.Move(camera.forward, GetJoystickAxis(0, 1) * SPEED * delta);

            camera.RotateY(GetJoystickAxis(0, 2) * ROTATE_SPEED * delta);
            camera.RotateX(-GetJoystickAxis(0, 3) * ROTATE_SPEED * delta);

            SPEED -= GetJoystickAxis(0, 4) * 50.0f * delta;
            SPEED += GetJoystickAxis(0, 5) * 50.0f * delta;

            if(SPEED < 0.0f)
            {
                SPEED = 0.0f;
            }

            Camera.Set(camera);
        }
    }
}