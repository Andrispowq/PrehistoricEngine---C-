using System;
using System.Runtime.InteropServices;

public class ExampleComponent
{
    public float some_value = 0.0f;

    public void Update(float delta)
    {
        Vector3f vector = new Vector3f(5.0f, 0.0f, 0.0f);
        vector = vector.Mul(delta);

        Callback.Move(vector);
    }

    public void Render()
    {
        some_value += 1.0f;
    }
}