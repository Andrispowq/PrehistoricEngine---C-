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
}