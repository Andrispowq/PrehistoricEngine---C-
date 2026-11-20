using System;

public class Logger : BaseComponent
{
    public override void OnInit()
    {
        Log("Logger initialised!\n");
    }

    public override void OnUpdate(float delta)
    {
        if(IsKeyPressed((int)Input.KEY_E))
        {
            Log("The E key was pressed!\n");
        }
    }
}