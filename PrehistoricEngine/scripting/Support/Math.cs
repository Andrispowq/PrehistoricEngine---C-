using System;

public class Vector3f
{
    public float x = 0.0f;
    public float y = 0.0f;
    public float z = 0.0f;

    public Vector3f() { }
    public Vector3f(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }
    public Vector3f(float v) { this.x = v; this.y = v; this.z = v; }

    public float Dot(Vector3f v) { return x * v.x + y * v.y + z + v.z; }
    public float Length() { return (float)Math.Sqrt((double)Dot(this)); }

    public Vector3f Add(Vector3f v) { return new Vector3f(x + v.x, y + v.y, z + v.z); }
    public Vector3f Add(float v) { return new Vector3f(x + v, y + v, z + v); }
    public Vector3f Sub(Vector3f v) { return new Vector3f(x - v.x, y - v.y, z - v.z); }
    public Vector3f Sub(float v) { return new Vector3f(x - v, y - v, z - v); }

    public Vector3f Mul(Vector3f v) { return new Vector3f(x * v.x, y * v.y, z * v.z); }
    public Vector3f Mul(float v) { return new Vector3f(x * v, y * v, z * v); }
    public Vector3f Div(Vector3f v) { return new Vector3f(x / v.x, y / v.y, z / v.z); }
    public Vector3f Div(float v) { return new Vector3f(x / v, y / v, z / v); }

    public static Vector3f operator +(Vector3f v) => v;
    public static Vector3f operator -(Vector3f v) => (v * -1);

    public static Vector3f operator +(Vector3f a, Vector3f b) => a.Add(b);
    public static Vector3f operator -(Vector3f a, Vector3f b) => a.Sub(b);
    public static Vector3f operator *(Vector3f a, Vector3f b) => a.Mul(b);
    public static Vector3f operator /(Vector3f a, Vector3f b) => a.Div(b);

    public static Vector3f operator +(Vector3f a, float b) => a.Add(b);
    public static Vector3f operator -(Vector3f a, float b) => a.Sub(b);
    public static Vector3f operator *(Vector3f a, float b) => a.Mul(b);
    public static Vector3f operator /(Vector3f a, float b) => a.Div(b);

    public static Vector3f operator +(float a, Vector3f b) => new Vector3f(a).Add(b);
    public static Vector3f operator -(float a, Vector3f b) => new Vector3f(a).Sub(b);
    public static Vector3f operator *(float a, Vector3f b) => new Vector3f(a).Mul(b);
    public static Vector3f operator /(float a, Vector3f b) => new Vector3f(a).Div(b);
}