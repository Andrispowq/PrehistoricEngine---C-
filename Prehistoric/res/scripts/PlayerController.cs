using System;

public class PlayerController : BaseComponent
{
    public static float MAX_SPEED = 20.0f;
    public static float ROTATE_SPEED = 180.0f;

    public static Vector3f forward = new Vector3f(0, 0, 1);
    public float current_speed = 0.0f;

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);
    }

    private float GetAcceleration(float curr_speed)
    {
        return 10.0f * (float)Math.Sin((curr_speed + 20) / 22);
    }

    public override void OnUpdate(float delta)
    {
        if (IsKeyPressed(Input.KEY_U))
        {
            current_speed += GetAcceleration(current_speed) * delta;
        }
        if (IsKeyPressed(Input.KEY_J))
        {
            current_speed -= 8.0f * delta;
        }       
        if (current_speed < 0) current_speed = 0;

        transform.position += forward * current_speed;
    }
}