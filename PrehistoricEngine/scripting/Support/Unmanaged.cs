using System;
using System.Text;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Prehistoric;

namespace Prehistoric
{
    public class Transform
    {
        public Vector3f position;
        public Vector3f rotation;
        public Vector3f scaling;

        public float[] ToRaw()
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

        public static Transform FromRaw(float[] data)
        {
            Transform transform = new Transform();
            transform.position = new Vector3f(data[0], data[1], data[2]);
            transform.rotation = new Vector3f(data[3], data[4], data[5]);
            transform.scaling = new Vector3f(data[6], data[7], data[8]);

            return transform;
        }

        public static Transform Get(IntPtr parent = default(IntPtr))
        {
            float[] buffer = new float[9];
            InternalCalls.GetTransform(buffer, parent);

            Transform ret = Transform.FromRaw(buffer);
            return ret;
        }

        public static void Set(Transform transform)
        {
            float[] buffer = transform.ToRaw();
            InternalCalls.SetTransform(buffer);
        }
    }

    public class Material
    {
        public Vector3f colour;
        public float metallic;
        public float roughness;
        public float occlusion;

        public float[] ToRaw()
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

        public static Material FromRaw(float[] data)
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

        public float[] ToRaw()
        {
            return material.ToRaw();
        }

        public static Renderer FromRaw(float[] data)
        {
            Renderer renderer = new Renderer();
            renderer.material = Material.FromRaw(data);

            return renderer;
        }

        public static Renderer Get()
        {
            float[] buffer = new float[6];
            InternalCalls.GetComponent("RendererComponent", buffer);

            Renderer ret = Renderer.FromRaw(buffer);
            return ret;
        }

        public static unsafe void Set(Renderer renderer)
        {
            float[] buffer = renderer.ToRaw();
            InternalCalls.SetComponent("RendererComponent", buffer);
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

        public float[] ToRaw()
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

        public static Camera FromRaw(float[] data)
        {
            Camera camera = new Camera();
            camera.position = new Vector3f(data[0], data[1], data[2]);
            camera.forward = new Vector3f(data[3], data[4], data[5]);
            camera.up = new Vector3f(data[6], data[7], data[8]);

            return camera;
        }

        public static Camera Get()
        {
            float[] buffer = new float[9];
            InternalCalls.GetComponent("CameraComponent", buffer);

            Console.WriteLine(buffer[0]);

            Camera ret = Camera.FromRaw(buffer);
            return ret;
        }

        public static void Set(Camera camera)
        {
            float[] buffer = camera.ToRaw();
            InternalCalls.SetComponent("CameraComponent", buffer);
        }
    }

    public class GameObject
    {
        public System.IntPtr pointer = (System.IntPtr)0;
        public String name = "Unnamed";

        public static GameObject GetSceneRoot()
        {
            GameObject result = new GameObject();
            result.name = "root";
            result.pointer = InternalCalls.GetSceneRoot();
            return result;
        }

        public static GameObject GetGameObject(string name, GameObject parent)
        {
            GameObject result = new GameObject();
            result.name = name;
            result.pointer = InternalCalls.GetGameObject(name, parent.pointer);
            return result;
        }

        public bool IsValid()
        {
            if (pointer == (System.IntPtr)0)
            {
                return false;
            }

            return true;
        }

        public Transform GetTransform()
        {
            if (IsValid())
                return Transform.Get(pointer);
            else
                return null;
        }
    }

    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool HasComponent(string name);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetComponent(string name, float[] data);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetComponent(string name, float[] data);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetTransform(float[] buffer, IntPtr parent = default(IntPtr));
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetTransform(float[] buffer);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ScriptLog(string fmt);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float InputCheck(int type, int val);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static IntPtr GetSceneRoot();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static IntPtr GetGameObject(string name, IntPtr parent);
    }
}