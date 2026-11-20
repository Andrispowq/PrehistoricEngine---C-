using System;
using Prehistoric;

public class ExampleComponent : BaseComponent
{
    public float ACCELERATION = 0.5f;
    public float VELOCITY = 0.0f;

    public override void OnInit()
    {
        transform.position = new Vector3f(0, 5, 0);

        if (HasComponent("RendererComponent"))
        {
            Renderer renderer = Renderer.Get();

            renderer.material.metallic = 0.5f;
            renderer.material.roughness = 1.0f;
            renderer.material.colour = new Vector3f(1, 0, 0);

            Renderer.Set(renderer);
        }

        Log("Initialised ExampleComponent!");
    }

    public override void OnUpdate(float delta)
    {
        VELOCITY += ACCELERATION * delta;

        transform.position.x += VELOCITY * delta;
    }
}