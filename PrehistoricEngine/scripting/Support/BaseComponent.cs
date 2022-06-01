using System;

public class BaseComponent
{
    public Transform transform;
    public Renderer renderer;

    public virtual void OnInit() { }
    public virtual void OnUpdate(float delta) { }
    public virtual void OnRender() { }

    public void Init()
    {
        unsafe
        {
            transform = Callback.GetTransform();
            renderer = Renderer.FromRaw((float*)Callback.GetComponent("RendererComponent"));
        }

        OnInit();

        unsafe
        {
            fixed (float* ptr = renderer.ToRaw())
            {
                Callback.SetComponent("RendererComponent", (void*)ptr);
                Callback.SetTransform(transform);
            }
        }
    }

    public void Update(float delta)
    {
        unsafe
        {
            transform = Callback.GetTransform();
            renderer = Renderer.FromRaw((float*)Callback.GetComponent("RendererComponent"));
        }

        OnUpdate(delta);

        unsafe
        {
            fixed (float* ptr = renderer.ToRaw())
            {
                Callback.SetComponent("RendererComponent", (void*)ptr);
                Callback.SetTransform(transform);
            }
        }
    }
    
    public void Render()
    {
        OnRender();
    }

    public void Log(string text)
    {
        Callback.Log(text);
    }

    //type -> 4 bits, pressed/held/released -> 3 bits, isAxis -> 1 bit, joystickID -> 4 bits, reserved -> 20 bits
    public bool IsKeyPressed(int key)
    {
        float ret = Callback.InputCheck(0x000000000, key);
        return (ret == 1.0f);
    }

    public bool IsKeyHeld(int key)
    {
        float ret = Callback.InputCheck(0x000000010, key);
        return (ret == 1.0f);
    }

    public bool IsMouseButtonPressed(int button)
    {
        float ret = Callback.InputCheck(0x000000001, button);
        return (ret == 1.0f);
    }

    public bool IsMouseButtonHeld(int button)
    {
        float ret = Callback.InputCheck(0x000000011, button);
        return (ret == 1.0f);
    }

    public float GetJoystickAxis(int joystickID, int axis)
    {
        int type = 0x000000082 | (joystickID << 8);

        float ret = Callback.InputCheck(type, axis);
        return ret;
    }

    public bool IsJoystickButtonPressed(int joystickID, int button)
    {
        int type = 0x000000002 | (joystickID << 8);

        float ret = Callback.InputCheck(type, button);
        return (ret == 1.0f);
    }
}