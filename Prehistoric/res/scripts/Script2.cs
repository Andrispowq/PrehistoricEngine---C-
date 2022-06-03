using System;

public class ExampleComponent : BaseComponent
{
    public float SPEED = 20.0f;
    public Vector3f DIRECTION = new Vector3f(1, 0, 0);

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);

        renderer.material.metallic = 1.0f;
        renderer.material.roughness = 0.5f;
        renderer.material.colour = new Vector3f(0, 0, 1);

        Log("Initialised ExampleComponent2!");
    }

    public override void OnUpdate(float delta)
    {
        transform.position.x -= GetJoystickAxis(0, 0) * SPEED * delta;
    }
}