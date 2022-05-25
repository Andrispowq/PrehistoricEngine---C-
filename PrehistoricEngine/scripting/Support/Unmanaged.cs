using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Explicit, Size = 16, CharSet = CharSet.Ansi)]
public class CallbackData
{
    [FieldOffset(0)] public int type = 0;
    [FieldOffset(8)] public unsafe void* data = (void*)0;
}

public class Callback
{
    [DllImport("__Internal", EntryPoint = "ScriptCallback")]
    public static extern unsafe void ScriptCallback([MarshalAs(UnmanagedType.LPStruct)] CallbackData data);

    public static unsafe void Move(Vector3f amount)
    {
        float[] values = { amount.x, amount.y, amount.z };

        fixed (float* ptr = &values[0])
        {
            CallbackData callback_data = new CallbackData();
            callback_data.type = 0;
            callback_data.data = (void*)ptr;

            ScriptCallback(callback_data);
        }
    }
}