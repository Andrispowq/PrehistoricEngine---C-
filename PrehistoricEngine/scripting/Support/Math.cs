using System;

public class Quaternion
{
    public float x = 0.0f;
    public float y = 0.0f;
    public float z = 0.0f;
    public float w = 0.0f;

    public Quaternion() { }
    public Quaternion(float x, float y, float z, float w) { this.x = x; this.y = y; this.z = z; this.w = w; }
    public Quaternion(float v) { this.x = v; this.y = v; this.z = v; this.w = v; }

    public Quaternion Conjugate() { return new Quaternion(-x, -y, -z, w); }

    public void print() { Console.WriteLine("[" + x + ", " + y + ", " + z + ", " + w +  "]"); }

    public Quaternion Add(Quaternion v) { return new Quaternion(x + v.x, y + v.y, z + v.z, w + v.z); }
    public Quaternion Add(float v) { return new Quaternion(x + v, y + v, z + v, w + v); }
    public Quaternion Sub(Quaternion v) { return new Quaternion(x - v.x, y - v.y, z - v.z, w - v.w); }
    public Quaternion Sub(float v) { return new Quaternion(x - v, y - v, z - v, w - v); }

    public Quaternion Mul(Quaternion r)
    {
        float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
        float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
        float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
        float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

        return new Quaternion(x_, y_, z_, w_);
    }

    public Quaternion Mul(Vector3f r)
    {
        float x_ = w * r.x + y * r.z - z * r.y;
        float y_ = w * r.y + z * r.x - x * r.z;
        float z_ = w * r.z + x * r.y - y * r.x;
        float w_ = -x * r.x - y * r.y - z * r.z;

        return new Quaternion(x_, y_, z_, w_);
    }

    public static Quaternion operator +(Quaternion v) => v;
    //public static Quaternion operator -(Quaternion v) => (v * -1);

    public static Quaternion operator +(Quaternion a, Quaternion b) => a.Add(b);
    public static Quaternion operator -(Quaternion a, Quaternion b) => a.Sub(b);
    public static Quaternion operator *(Quaternion a, Quaternion b) => a.Mul(b);
    //public static Quaternion operator /(Quaternion a, Quaternion b) => a.Div(b);

    public static Quaternion operator *(Quaternion a, Vector3f b) => a.Mul(b);

    public static Quaternion operator +(Quaternion a, float b) => a.Add(b);
    public static Quaternion operator -(Quaternion a, float b) => a.Sub(b);
    //public static Quaternion operator *(Quaternion a, float b) => a.Mul(b);
    //public static Quaternion operator /(Quaternion a, float b) => a.Div(b);

    public static Quaternion operator +(float a, Quaternion b) => new Quaternion(a).Add(b);
    public static Quaternion operator -(float a, Quaternion b) => new Quaternion(a).Sub(b);
    public static Quaternion operator *(float a, Quaternion b) => new Quaternion(a).Mul(b);
    //public static Quaternion operator /(float a, Quaternion b) => new Quaternion(a).Div(b);
}

public class Vector3f
{
    public float x = 0.0f;
    public float y = 0.0f;
    public float z = 0.0f;

    public Vector3f() { }
    public Vector3f(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }
    public Vector3f(float v) { this.x = v; this.y = v; this.z = v; }

    public float Dot(Vector3f v) { return x * v.x + y * v.y + z * v.z; }
    public float Length() { return (float)Math.Sqrt((double)Dot(this)); }

    public Vector3f normalise() { return this / Length(); }

    public void print() { Console.WriteLine("[" + x + ", " + y + ", " + z + "]"); }

    public Vector3f rotate(Vector3f axis, float angle)
    {
        Vector3f result = new Vector3f();

        float sinHalfAngle = (float)Math.Sin((angle / 2) * 3.141592653589832 / 180.0f);
        float cosHalfAngle = (float)Math.Cos((angle / 2) * 3.141592653589832 / 180.0f);

        float rx = axis.x * sinHalfAngle;
        float ry = axis.y * sinHalfAngle;
        float rz = axis.z * sinHalfAngle;
        float rw = cosHalfAngle;

        Quaternion rotation = new Quaternion(rx, ry, rz, rw);
        Quaternion conjugate = rotation.Conjugate();

        Quaternion w = rotation * this * conjugate;

        result.x = w.x;
        result.y = w.y;
        result.z = w.z;

        return result;
    }

    public Vector3f cross(Vector3f v)
    {
        float x_ = y * v.z - z * v.y;
        float y_ = z * v.x - x * v.z;
        float z_ = x * v.y - y * v.x;

	    return new Vector3f(x_, y_, z_);
    }

    public Vector3f reflect(Vector3f normal)
    {
	    return this - normal * 2 * Dot(normal);
    }

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