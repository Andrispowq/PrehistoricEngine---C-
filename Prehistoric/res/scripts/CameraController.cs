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

            camera.Move(camera.GetLeft(), GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_LEFT_X) * SPEED * delta);
            camera.Move(camera.forward, GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_LEFT_Y) * SPEED * delta);

            camera.RotateY(GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_RIGHT_X) * ROTATE_SPEED * delta);
            camera.RotateX(-GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_RIGHT_Y) * ROTATE_SPEED * delta);

            SPEED -= GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_LEFT_TRIGGER) * 50.0f * delta;
            SPEED += GetJoystickAxis(InputKey.JOYSTICK_0, InputKey.GAMEPAD_AXIS_RIGHT_TRIGGER) * 50.0f * delta;

            if(SPEED < 0.0f)
            {
                SPEED = 0.0f;
            }

            Camera.Set(camera);
        }
    }
}