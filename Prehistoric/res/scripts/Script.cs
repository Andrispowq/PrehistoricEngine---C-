using System;

public class ExampleComponent : BaseComponent
{
    public const float SPEED = 5.0f;

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);
        Log("Initialised ExampleComponent!");
    }

    public override void OnUpdate(float delta)
    {
        transform.position = transform.position.Add(new Vector3f(SPEED * delta, 0.0f, 0.0f));
    }
}