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

    public static Transform GetTransform()
    {
        unsafe
        {
            CallbackData callback_data = new CallbackData();
            callback_data.type = 0x3;
            callback_data.data = (void*)0;

            ScriptCallback(callback_data);
            float* data = (float*)callback_data.data;
               
            Transform transform = new Transform();               
            transform.position = new Vector3f(data[0], data[1], data[2]);               
            transform.rotation = new Vector3f(data[3], data[4], data[5]);
            transform.scaling = new Vector3f(data[6], data[7], data[8]);

            return transform;
        }
    }

    public static void SetTransform(Transform transform)
    {
        unsafe
        {
            float[] fdata = new float[9];
            fdata[0] = transform.position.x;
            fdata[1] = transform.position.y;
            fdata[2] = transform.position.z;
            fdata[3] = transform.rotation.x;
            fdata[4] = transform.rotation.y;
            fdata[5] = transform.rotation.z;
            fdata[6] = transform.scaling.x;
            fdata[7] = transform.scaling.y;
            fdata[8] = transform.scaling.z;

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
}