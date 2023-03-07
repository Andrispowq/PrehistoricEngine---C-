using System;
using System.Text;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Explicit, Size = 16, CharSet = CharSet.Ansi)]
public class CallbackData
{
    [FieldOffset(0)] public int type = 0;
    [FieldOffset(8)] public unsafe void* data = (void*)0;
}

public class Transform
{
    public Vector3f position;
    public Vector3f rotation;
    public Vector3f scaling;

    public unsafe float[] ToRaw()
    {
        float[] data = new float[9];
        data[0] = position.x;
        data[1] = position.y;
        data[2] = position.z;
        data[3] = rotation.x;
        data[4] = rotation.y;
        data[5] = rotation.z;
        data[6] = scaling.x;
        data[7] = scaling.y;
        data[8] = scaling.z;

        return data;
    }

    public static unsafe Transform FromRaw(float* data)
    {
        Transform transform = new Transform();
        transform.position = new Vector3f(data[0], data[1], data[2]);
        transform.rotation = new Vector3f(data[3], data[4], data[5]);
        transform.scaling = new Vector3f(data[6], data[7], data[8]);

        return transform;
    }

    public static Transform Get()
    {
        return Callback.GetTransform();
    }

    public static void Set(Transform transform)
    {
        Callback.SetTransform(transform);
    }
}

public class Material
{
    public Vector3f colour;
    public float metallic;
    public float roughness;
    public float occlusion;

    public unsafe float[] ToRaw()
    {
        float[] data = new float[6];
        data[0] = colour.x;
        data[1] = colour.y;
        data[2] = colour.z;
        data[3] = metallic;
        data[4] = roughness;
        data[5] = occlusion;

        return data;
    }

    public static unsafe Material FromRaw(float* data)
    {
        Material material = new Material();
        material.colour = new Vector3f(data[0], data[1], data[2]);
        material.metallic = data[3];
        material.roughness = data[4];
        material.occlusion = data[5];

        return material;
    }
}

public class Renderer
{
    public Material material;

    public unsafe float[] ToRaw()
    {
        return material.ToRaw();
    }

    public static unsafe Renderer FromRaw(float* data)
    {
        Renderer renderer = new Renderer();
        renderer.material = Material.FromRaw(data);

        return renderer;
    }

    public static unsafe Renderer Get()
    {
        return Renderer.FromRaw((float*)Callback.GetComponent("RendererComponent"));
    }

    public static unsafe void Set(Renderer renderer)
    {
        fixed (float* ptr = renderer.ToRaw())
        {
            Callback.SetComponent("RendererComponent", (void*)ptr);
        }
    }
}

public class Camera
{
    private static Vector3f yAxis = new Vector3f(0, 1, 0);

    public Vector3f position;
    public Vector3f forward;
    public Vector3f up;

    public Vector3f GetLeft()
    {
        return forward.cross(up);
    }

    public void Move(Vector3f dir, float am)
    {
        position += dir * am;
    }

    public void RotateX(float am)
    {
        Vector3f hAxis = yAxis.cross(forward).normalise();
        forward = forward.rotate(hAxis, am).normalise();
        up = forward.cross(hAxis).normalise();
    }

    public void RotateY(float am)
    {
        Vector3f hAxis = yAxis.cross(forward).normalise();
        forward = forward.rotate(yAxis, am).normalise();
        up = forward.cross(hAxis).normalise();
    }

    public unsafe float[] ToRaw()
    {
        float[] data = new float[9];
        data[0] = position.x;
        data[1] = position.y;
        data[2] = position.z;
        data[3] = forward.x;
        data[4] = forward.y;
        data[5] = forward.z;
        data[6] = up.x;
        data[7] = up.y;
        data[8] = up.z;

        return data;
    }

    public static unsafe Camera FromRaw(float* data)
    {
        Camera camera = new Camera();
        camera.position = new Vector3f(data[0], data[1], data[2]);
        camera.forward = new Vector3f(data[3], data[4], data[5]);
        camera.up = new Vector3f(data[6], data[7], data[8]);

        return camera;
    }

    public static unsafe Camera Get()
    {
        return Camera.FromRaw((float*)Callback.GetComponent("CameraComponent"));
    }

    public static unsafe void Set(Camera camera)
    {
        fixed (float* ptr = camera.ToRaw())
        {
            Callback.SetComponent("CameraComponent", (void*)ptr);
        }
    }
}

public class GameObject
{
    public System.IntPtr pointer = (System.IntPtr)0;
    public String name = "Unnamed";
    
    public bool IsValid()
    {
        if(pointer == (System.IntPtr)0)
        {
            return false;
        }

        return true;
    }

    public Transform GetTransform()
    {
        if (IsValid())
            return Callback.GetTransform(this);
        else
            return null;
    }
}

public class Callback
{
    [DllImport("__Internal", EntryPoint = "ScriptCallback")]
    public static extern unsafe void ScriptCallback([MarshalAs(UnmanagedType.LPStruct)] CallbackData data);

    public static unsafe bool HasComponent(string name)
    {
        fixed(char* ptr = name)
        {
            CallbackData callback_data = new CallbackData();
            callback_data.type = 0x0;
            callback_data.data = (void*)ptr;

            ScriptCallback(callback_data);

            bool res_int = *(bool*)callback_data.data;
            return res_int;
        }
    }

    public static unsafe void* GetComponent(string name)
    {
        fixed (char* ptr = name)
        {
            CallbackData callback_data = new CallbackData();
            callback_data.type = 0x1;
            callback_data.data = (void*)ptr;

            ScriptCallback(callback_data);
            return callback_data.data;
        }
    }

    public static unsafe void SetComponent(string name, void* data)
    {
        fixed (char* ptr = name)
        {
            System.IntPtr[] ptrs = new System.IntPtr[2];
            ptrs[0] = (System.IntPtr)ptr;
            ptrs[1] = (System.IntPtr)data;

            fixed (System.IntPtr* _ptrs = ptrs)
            {
                CallbackData callback_data = new CallbackData();
                callback_data.type = 0x2;
                callback_data.data = (void*)_ptrs;

                ScriptCallback(callback_data);
            }
        }
    }

    public static Transform GetTransform(GameObject obj = null)
    {
        unsafe
        {
            CallbackData callback_data = new CallbackData();
            callback_data.type = 0x3;

            System.IntPtr[] ptrs = new System.IntPtr[1];
            if (obj == null)
            {
                callback_data.data = (void*)0;
            }
            else
            {
                ptrs[0] = (System.IntPtr)obj.pointer;

                fixed (System.IntPtr* _ptrs = ptrs)
                {
                    callback_data.data = (void*)_ptrs;
                }
            }

            ScriptCallback(callback_data);
            float* data = (float*)callback_data.data;

            return Transform.FromRaw(data);
        }
    }

    public static void SetTransform(Transform transform)
    {
        unsafe
        {
            float[] fdata = transform.ToRaw();

            fixed (float* data = fdata)
            {
                CallbackData callback_data = new CallbackData();
                callback_data.type = 0x4;
                callback_data.data = (void*)data;

                ScriptCallback(callback_data);
            }
        }
    }

    public static void Log(string text)
    {
        unsafe
        {
            byte[] arr = Encoding.ASCII.GetBytes(text);

            fixed(byte* text_ptr = arr)
            {
                CallbackData callback_data = new CallbackData();
                callback_data.type = 0x5;
                callback_data.data = (void*)text_ptr;

                ScriptCallback(callback_data);
            }
        }
    }

    public static float InputCheck(int input_type, int value)
    {
        unsafe
        {
            int[] arr = new int[2];
            arr[0] = input_type;
            arr[1] = value;

            fixed (int* ptr = arr)
            {
                CallbackData callback_data = new CallbackData();
                callback_data.type = 0x6;
                callback_data.data = (void*)ptr;

                ScriptCallback(callback_data);
                float val = *(float*)callback_data.data;
                return val;
            }
        }
    }

    public static GameObject GetGameObject(string name, GameObject parent = null)
    {
        unsafe
        {
            System.IntPtr[] ptrs = new System.IntPtr[2];

            if (parent == null)
            {
                ptrs[0] = (System.IntPtr)0;
            }
            else
            {
                ptrs[0] = (System.IntPtr)parent.pointer;
            }

            fixed (char* name_bytes = name)
            {
                ptrs[1] = (System.IntPtr)name_bytes;
            }

            fixed (System.IntPtr* _ptrs = ptrs)
            {
                CallbackData callback_data = new CallbackData();
                callback_data.type = 0x7;
                callback_data.data = (void*)_ptrs;

                ScriptCallback(callback_data);

                GameObject result = new GameObject();
                result.name = name;
                result.pointer = (System.IntPtr)callback_data.data;
                return result;
            }
        }
    }
}