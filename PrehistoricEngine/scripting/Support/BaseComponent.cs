using System;

public class BaseComponent
{
    public Transform transform;

    public virtual void OnInit() { }
    public virtual void OnUpdate(float delta) { }
    public virtual void OnRender() { }

    public void Init()
    {
        transform = Callback.GetTransform();

        OnInit();

        Callback.SetTransform(transform);
    }

    public void Update(float delta)
    {
        transform = Callback.GetTransform();

        OnUpdate(delta);

        Callback.SetTransform(transform);
    }
    
    public void Render()
    {
        OnRender();
    }

    public void Log(string text)
    {
        Callback.Log(text);
    }
}