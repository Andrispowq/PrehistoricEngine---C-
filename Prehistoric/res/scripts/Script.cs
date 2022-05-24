using System;
using System.Runtime.InteropServices;

public class ExampleComponent
{
    [DllImport("__Internal", EntryPoint = "ScriptCallback")]
    public static extern void ScriptCallback(float val);

    public float some_value = 0.0f;

    public void Update(float delta)
    {
        some_value += delta;
    }

    public void Render()
    {
        ScriptCallback(0.5f);
    }
}