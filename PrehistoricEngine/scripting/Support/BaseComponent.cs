using System;
using Prehistoric;

public class BaseComponent
{
    public Transform transform;

    public virtual void OnInit() { }
    public virtual void OnUpdate(float delta) { }
    public virtual void OnRender() { }

    public void Init()
    {
        transform = Transform.Get();

        OnInit();

        Transform.Set(transform);
    }

    public void Update(float delta)
    {
        transform = Transform.Get();

        OnUpdate(delta);

        Transform.Set(transform);
    }
    
    public void Render()
    {
        OnRender();
    }

    public void Log(string text)
    {
        InternalCalls.ScriptLog(text);
    }

    public bool HasComponent(string name)
    {
        return InternalCalls.HasComponent(name);
    }

    //type -> 4 bits, pressed/held/released -> 3 bits, isAxis -> 1 bit, joystickID -> 4 bits, reserved -> 20 bits
    public bool IsKeyPressed(int key)
    {
        float ret = InternalCalls.InputCheck(0x000000000, key);
        return (ret == 1.0f);
    }

    public bool IsKeyHeld(int key)
    {
        float ret = InternalCalls.InputCheck(0x000000010, key);
        return (ret == 1.0f);
    }

    public bool IsMouseButtonPressed(int button)
    {
        float ret = InternalCalls.InputCheck(0x000000001, button);
        return (ret == 1.0f);
    }

    public bool IsMouseButtonHeld(int button)
    {
        float ret = InternalCalls.InputCheck(0x000000011, button);
        return (ret == 1.0f);
    }

    public float GetJoystickAxis(int joystickID, int axis)
    {
        int type = 0x000000082 | (joystickID << 8);

        float ret = InternalCalls.InputCheck(type, axis);
        if(Math.Abs(ret) < 0.2f)
        {
            return 0.0f;
        }

        return ret;
    }

    public bool IsJoystickButtonPressed(int joystickID, int button)
    {
        int type = 0x000000002 | (joystickID << 8);

        float ret = InternalCalls.InputCheck(type, button);
        return (ret == 1.0f);
    }
}