using System;

public class ExampleComponent
{
    public float some_value;

    public static void Update(float delta)
    {
        Console.Write("Updated component with delta time ");
        Console.Write(delta);
        Console.WriteLine();
    }
}