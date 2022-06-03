using System;

public class ExampleComponent : BaseComponent
{
    public float SPEED = 20.0f;
    public Vector3f DIRECTION = new Vector3f(1, 0, 0);

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);

        renderer.material.metallic = 0.5f;
        renderer.material.roughness = 1.0f;
        //renderer.material.colour = new Vector3f(1, 0, 0);

        Log("Initialised ExampleComponent!");
        //Log("Initialised ExampleComponent again!");
    }

    public override void OnUpdate(float delta)
    {
        //renderer.material.colour = new Vector3f(1, 0, 0);
        transform.position.x += GetJoystickAxis(0, 0) * SPEED * delta;
    }
}