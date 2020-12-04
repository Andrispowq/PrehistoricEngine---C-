#ifndef MATH_H
#define MATH_H

#include "engine/prehistoric/core/util/Includes.hpp"
#include "engine/platform/Platform.h"

#include <xmmintrin.h>

#define PI 3.14159265358979323846264338327950288419716939

template<typename T>
static T ToDegrees(T x)
{
	return static_cast<T>((x / PI) * 180.0);
}

template<typename T>
static T ToRadians(T x)
{
	return static_cast<T>((x * PI) / 180.0);
}

//On x64 Windows we use SIMD to offer faster math
#if defined(PR_WINDOWS_64) && defined(PR_FAST_MATH) 

class Vector2f
{
public:
	Vector2f(const Vector2f& v) { this->x = v.x; this->y = v.y; }

	Vector2f(const float& x, const float& y) : x(x), y(y) {}
	Vector2f(const float& v) : x(v), y(v) {}
	Vector2f(const __m128& v) : reg(v) {}
	Vector2f() : x(0), y(0) {}

	inline Vector2f operator+(const Vector2f& v) const { return _mm_add_ps(reg, v.reg); }
	inline Vector2f operator-(const Vector2f& v) const { return _mm_sub_ps(reg, v.reg); }
	inline Vector2f operator*(const Vector2f& v) const { return _mm_mul_ps(reg, v.reg); }
	inline Vector2f operator/(const Vector2f& v) const { return _mm_div_ps(reg, v.reg); }

	inline Vector2f operator+(const float& v) const { return _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator-(const float& v) const { return _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator*(const float& v) const { return _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator/(const float& v) const { return _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline Vector2f operator+=(const Vector2f& v) { return reg = _mm_add_ps(reg, v.reg); }
	inline Vector2f operator-=(const Vector2f& v) { return reg = _mm_sub_ps(reg, v.reg); }
	inline Vector2f operator*=(const Vector2f& v) { return reg = _mm_mul_ps(reg, v.reg); }
	inline Vector2f operator/=(const Vector2f& v) { return reg = _mm_div_ps(reg, v.reg); }

	inline Vector2f operator+=(const float& v) { return reg = _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator-=(const float& v) { return reg = _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator*=(const float& v) { return reg = _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector2f operator/=(const float& v) { return reg = _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline float operator[](const int& index) const
	{
		return reg.m128_f32[index];
	}

	inline bool operator<(const Vector2f& v) const { return x < v.x && y < v.y; }
	inline bool operator>(const Vector2f& v) const { return x > v.x && y > v.y; }
	inline bool operator<=(const Vector2f& v) const { return x <= v.x && y <= v.y; }
	inline bool operator>=(const Vector2f& v) const { return x >= v.x && y >= v.y; }
	inline bool operator==(const Vector2f& v) const { return x == v.x && y == v.y; }
	inline bool operator!=(const Vector2f& v) const { return !(operator==(v)); }

	inline float length() const { return sqrt(this->dot(*this)); }
	inline float lengthSquared() const { return this->dot(*this); }
	inline Vector2f normalise() const { return *this / length(); }
	inline float dot(const Vector2f& v) const { return x * v.x + y * v.y; }

	inline Vector2f abs() const;

	Vector2f lerp(const Vector2f& b, const float& t, const bool& invert) const;
	Vector2f lerp(const Vector2f& b, const Vector2f& t, const bool& invert) const;

	Vector2f swap() const;
	
	Vector2f negate() const { return _mm_sub_ps(_mm_set_ss(0.0), reg); };
	Vector2f negated() { return reg = _mm_sub_ps(_mm_set_ss(0.0), reg); };

	void print() const { PR_LOG_MESSAGE("[ %f, %f ]\n", (float)x, (float)y); }
	
	constexpr static uint64_t size() { return 2 * sizeof(float); }
public:
	union
	{
		struct { float x, y; };
		struct { float r, g; };
		struct { float s, t; };
		__m128 reg;
	};
};

class Vector3f
{
public:
	Vector3f(const Vector3f& v) { this->x = v.x; this->y = v.y; this->z = v.z; }

	Vector3f(const Vector2f& xy, const float& z) : x(xy.x), y(xy.y), z(z) {}
	Vector3f(const float& x, const Vector2f& yz) : x(x), y(yz.x), z(yz.y) {}
	Vector3f(const float& x, float y, float z) : x(x), y(y), z(z) {}
	Vector3f(const float& v) : x(v), y(v), z(v) {}
	Vector3f(const __m128& v) : reg(v) {}
	Vector3f() : x(0), y(0), z(0) {}

	~Vector3f() {}

	inline Vector3f operator+(const Vector3f& v) const { return _mm_add_ps(reg, v.reg); }
	inline Vector3f operator-(const Vector3f& v) const { return _mm_sub_ps(reg, v.reg); }
	inline Vector3f operator*(const Vector3f& v) const { return _mm_mul_ps(reg, v.reg); }
	inline Vector3f operator/(const Vector3f& v) const { return _mm_div_ps(reg, v.reg); }

	inline Vector3f operator+(const float& v) const { return _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator-(const float& v) const { return _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator*(const float& v) const { return _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator/(const float& v) const { return _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline Vector3f operator+=(const Vector3f& v) { return reg = _mm_add_ps(reg, v.reg); }
	inline Vector3f operator-=(const Vector3f& v) { return reg = _mm_sub_ps(reg, v.reg); }
	inline Vector3f operator*=(const Vector3f& v) { return reg = _mm_mul_ps(reg, v.reg); }
	inline Vector3f operator/=(const Vector3f& v) { return reg = _mm_div_ps(reg, v.reg); }

	inline Vector3f operator+=(const float& v) { return reg = _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator-=(const float& v) { return reg = _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator*=(const float& v) { return reg = _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector3f operator/=(const float& v) { return reg = _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline float operator[](const int& index) const
	{
		return reg.m128_f32[index];
	}

	inline bool operator<(const Vector3f& v) const { return x < v.x&& y < v.y && z < v.z; }
	inline bool operator>(const Vector3f& v) const { return x > v.x && y > v.y && z > v.z; }
	inline bool operator<=(const Vector3f& v) const { return x <= v.x && y <= v.y && z <= v.z; }
	inline bool operator>=(const Vector3f& v) const { return x >= v.x && y >= v.y && z >= v.z; }
	inline bool operator==(const Vector3f& v) const { return x == v.x && y == v.y && z == v.z; }
	inline bool operator!=(const Vector3f& v) const { return !(operator==(v)); }

	inline float length() const { return sqrt(this->dot(*this)); }
	inline float lengthSquared() const { return this->dot(*this); }
	inline Vector3f normalise() const { return *this / length(); }
	inline float dot(const Vector3f& v) const { return x * v.x + y * v.y + z * v.z; }

	inline Vector3f abs() const;

	Vector3f cross(const Vector3f& normal) const;
	Vector3f reflect(const Vector3f& normal) const;
	Vector3f refract(const Vector3f& normal, const float& eta) const;

	Vector3f lerp(const Vector3f& b, const float& t) const;
	Vector3f lerp(const Vector3f& b, const Vector3f& t) const;

	Vector3f rotate(const Vector3f& angles) const;
	Vector3f rotate(const Vector3f& axis, const float& angle) const;

	Vector3f negate() const { return _mm_sub_ps(_mm_set_ss(0.0), reg); };
	Vector3f negated() { return reg = _mm_sub_ps(_mm_set_ss(0.0), reg); };

	Vector2f xy() { return Vector2f(x, y); }

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f ]\n", (float)x, (float)y, (float)z); }

	constexpr static uint64_t size() { return 3 * sizeof(float); }
public:
	union
	{
		struct { float x, y, z; };
		struct { float r, g, b; };
		struct { float s, t, r; };
		__m128 reg;
	};
};

//class Quaternionf;

class Vector4f
{
public:
	Vector4f(const Vector4f& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }
	//Vector4f(const Quaternionf& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }

	Vector4f(const float* v)
	{
		if (&(v[3]) == nullptr)
			PR_LOG_ERROR("You try to create a Vector4f object from an array of float with less then 4 members. floathe array's memory address: %i\n", v);

		this->x = v[0];
		this->y = v[1];
		this->z = v[2];
		this->w = v[3];
	}

	Vector4f(const Vector3f& xyz, const float& w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	Vector4f(const float& x, const Vector3f& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
	Vector4f(const Vector2f& xy, const float& z, const float& w) : x(xy.x), y(xy.y), z(w), w(w) {}
	Vector4f(const Vector2f& xy, const Vector2f& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	Vector4f(const float& x, const Vector2f& yz, const float& w) : x(x), y(yz.x), z(yz.y), w(w) {}
	Vector4f(const float& x, const float& y, const Vector2f& zw) : x(x), y(y), z(zw.x), w(zw.y) {}
	Vector4f(const float& x, const float& y, const float& z, const float& w) : x(x), y(y), z(z), w(w) {}
	Vector4f(const float& v) : x(v), y(v), z(v), w(v) {}
	Vector4f(const __m128& v) : reg(v) {}
	Vector4f() : x(0), y(0), z(0), w(0) {}

	~Vector4f() {}

	inline Vector4f operator+(const Vector4f& v) const { return _mm_add_ps(reg, v.reg); }
	inline Vector4f operator-(const Vector4f& v) const { return _mm_sub_ps(reg, v.reg); }
	inline Vector4f operator*(const Vector4f& v) const { return _mm_mul_ps(reg, v.reg); }
	inline Vector4f operator/(const Vector4f& v) const { return _mm_div_ps(reg, v.reg); }

	inline Vector4f operator+(const float& v) const { return _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator-(const float& v) const { return _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator*(const float& v) const { return _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator/(const float& v) const { return _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline Vector4f operator+=(const Vector4f& v) { return reg = _mm_add_ps(reg, v.reg); }
	inline Vector4f operator-=(const Vector4f& v) { return reg = _mm_sub_ps(reg, v.reg); }
	inline Vector4f operator*=(const Vector4f& v) { return reg = _mm_mul_ps(reg, v.reg); }
	inline Vector4f operator/=(const Vector4f& v) { return reg = _mm_div_ps(reg, v.reg); }

	inline Vector4f operator+=(const float& v) { return reg = _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator-=(const float& v) { return reg = _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator*=(const float& v) { return reg = _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Vector4f operator/=(const float& v) { return reg = _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline float operator[](const int& index) const
	{
		return reg.m128_f32[index];
	}

	inline bool operator<(const Vector4f& v) const { return x < v.x&& y < v.y && z < v.z&& w < v.w; }
	inline bool operator>(const Vector4f& v) const { return x > v.x && y > v.y && z > v.z && w > v.w; }
	inline bool operator<=(const Vector4f& v) const { return x <= v.x && y <= v.y && z <= v.z && w <= v.w; }
	inline bool operator>=(const Vector4f& v) const { return x >= v.x && y >= v.y && z >= v.z && w >= v.w; }
	inline bool operator==(const Vector4f& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
	inline bool operator!=(const Vector4f& v) const { return !(operator==(v)); }

	inline float length() const { return sqrt(this->dot(*this)); }
	inline float lengthSquared() const { return this->dot(*this); }
	inline Vector4f normalise() const { return *this / length(); }
	inline float dot(const Vector4f& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

	inline Vector4f abs() const;

	Vector4f lerp(const Vector4f& b, const float& t) const;
	Vector4f lerp(const Vector4f& b, const Vector4f& t) const;

	Vector4f negate() const { return _mm_sub_ps(_mm_set_ss(0.0), reg); };
	Vector4f negated() { return reg = _mm_sub_ps(_mm_set_ss(0.0), reg); };

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f, %f ]\n", (float)x, (float)y, (float)z, (float)w); }

	Vector3f xyz() const { return Vector3f(x, y, z); }

	constexpr static uint64_t size() { return 4 * sizeof(float); }
public:
	union
	{
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		__m128 reg;
	};
};

class Quaternionf
{
public:
	Quaternionf(const Quaternionf& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }
	Quaternionf(const Vector4f& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }

	Quaternionf(const float* v)
	{
		if (&(v[3]) == nullptr)
			PR_LOG_ERROR("You try to create a Quaternionf object from an array of float with less then 4 members. floathe array's memory address: %i\n", v);

		this->x = v[0];
		this->y = v[1];
		this->z = v[2];
		this->w = v[3];
	}

	Quaternionf(const Vector3f& xyz, const float& w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	Quaternionf(const float& x, const Vector3f& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
	Quaternionf(const Vector2f& xy, const float& z, const float& w) : x(xy.x), y(xy.y), z(w), w(w) {}
	Quaternionf(const Vector2f& xy, const Vector2f& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	Quaternionf(const float& x, const Vector2f& yz, const float& w) : x(x), y(yz.x), z(yz.y), w(w) {}
	Quaternionf(const float& x, const float& y, const Vector2f& zw) : x(x), y(y), z(zw.x), w(zw.y) {}
	Quaternionf(const float& x, const float& y, const float& z, const float& w) : x(x), y(y), z(z), w(w) {}
	Quaternionf(const float& v) : x(v), y(v), z(v), w(v) {}
	Quaternionf(const __m128& v) : reg(v) {}
	Quaternionf() : x(0), y(0), z(0), w(0) {}

	~Quaternionf() {}

	inline Quaternionf operator+(const Vector4f& v) const { return _mm_add_ps(reg, v.reg); }
	inline Quaternionf operator-(const Vector4f& v) const { return _mm_sub_ps(reg, v.reg); }
	inline Quaternionf operator+(const Quaternionf& v) const { return _mm_add_ps(reg, v.reg); }
	inline Quaternionf operator-(const Quaternionf& v) const { return _mm_sub_ps(reg, v.reg); }

	inline Quaternionf operator+(const float& v) const { return _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator-(const float& v) const { return _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator*(const float& v) const { return _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator/(const float& v) const { return _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline Quaternionf operator+=(const Vector4f& v) { return reg = _mm_add_ps(reg, v.reg); }
	inline Quaternionf operator-=(const Vector4f& v) { return reg = _mm_sub_ps(reg, v.reg); }
	inline Quaternionf operator+=(const Quaternionf& v) { return reg = _mm_add_ps(reg, v.reg); }
	inline Quaternionf operator-=(const Quaternionf& v) { return reg = _mm_sub_ps(reg, v.reg); }

	inline Quaternionf operator+=(const float& v) { return reg = _mm_add_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator-=(const float& v) { return reg = _mm_sub_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator*=(const float& v) { return reg = _mm_mul_ps(reg, _mm_set_ps(v, v, v, v)); }
	inline Quaternionf operator/=(const float& v) { return reg = _mm_div_ps(reg, _mm_set_ps(v, v, v, v)); }

	inline Quaternionf Conjugate() const { return Quaternionf(-x, -y, -z, w); }

	Quaternionf operator*(const Quaternionf& v) const;
	Quaternionf operator*(const Vector4f& v) const;
	Quaternionf operator*(const Vector3f& v) const;

	Quaternionf operator*=(const Quaternionf& v);
	Quaternionf operator*=(const Vector4f& v);
	Quaternionf operator*=(const Vector3f& v);

	inline float operator[](const int& index) const
	{
		return reg.m128_f32[index];
	}

	inline bool operator<(const Quaternionf& v) const { return x < v.x && y < v.y && z < v.z && w < v.w; }
	inline bool operator>(const Quaternionf& v) const { return x > v.x && y > v.y && z > v.z && w > v.w; }
	inline bool operator<=(const Quaternionf& v) const { return x <= v.x && y <= v.y && z <= v.z && w <= v.w; }
	inline bool operator>=(const Quaternionf& v) const { return x >= v.x && y >= v.y && z >= v.z && w >= v.w; }
	inline bool operator==(const Quaternionf& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
	inline bool operator!=(const Quaternionf& v) const { return !(operator==(v)); }

	inline float length() const { return sqrt(this->dot(*this)); }
	inline float lengthSquared() const { return this->dot(*this); }
	inline Quaternionf normalise() const { return *this / length(); }
	inline float dot(const Quaternionf& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

	inline Quaternionf abs() const;

	Quaternionf lerp(const Quaternionf& b, const float& t) const;
	Quaternionf lerp(const Quaternionf& b, const Quaternionf& t) const;

	Quaternionf negate() const { return _mm_sub_ps(_mm_set_ss(0.0), reg); };
	Quaternionf negated() { return reg = _mm_sub_ps(_mm_set_ss(0.0), reg); };

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f, %f ]", (float)x, (float)y, (float)z, (float)w); }
public:
	union
	{
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		__m128 reg;
	};
};

class Matrix4f
{
public:
	Matrix4f(const Matrix4f& v);
	Matrix4f(Matrix4f&& v) noexcept;

	Matrix4f(const Vector4f& v);
	Matrix4f();

	~Matrix4f();

	Matrix4f& operator=(const Matrix4f& v);
	Matrix4f& operator=(Matrix4f&& v) noexcept;

	inline Matrix4f operator+(const Matrix4f& v) const;
	inline Matrix4f operator-(const Matrix4f& v) const;

	inline Matrix4f operator+=(const Matrix4f& v);
	inline Matrix4f operator-=(const Matrix4f& v);

	inline Matrix4f operator+(const Vector4f& v) const;
	inline Matrix4f operator-(const Vector4f& v) const;

	inline Matrix4f operator+=(const Vector4f& v);
	inline Matrix4f operator-=(const Vector4f& v);

	inline Matrix4f operator+(const float& v) const;
	inline Matrix4f operator-(const float& v) const;

	inline Matrix4f operator+=(const float& v);
	inline Matrix4f operator-=(const float& v);

	inline Matrix4f operator*(const Matrix4f& v) const;
	inline Matrix4f operator*=(const Matrix4f& v);

	Vector4f operator*(const Vector4f& v) const;

	inline bool operator==(const Matrix4f& v) const;

	std::array<float, 4> operator[](int index) const;

	static Matrix4f Identity();
	static Matrix4f Zero();

	static Matrix4f Translation(const Vector3f& position);
	static Matrix4f Rotation(const Vector3f& rotation);
	static Matrix4f Scaling(const Vector3f& scale);

	static Matrix4f Transformation(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scaling);
	static Matrix4f PerspectiveProjection(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane);
	static Matrix4f View(const Vector3f& forward, const Vector3f& up);

	std::array<float, 4> const getRow(int i) const;

	void print() const
	{
		PR_LOG_MESSAGE("Matrix: \n");

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				PR_LOG_MESSAGE("\t%f", m[j * 4 + i]);
			}

			PR_LOG_MESSAGE("\n");
		}
	}
private:
	void clear();
public:
	union
	{
		float* m;
		__m128* coloumns;
	};
};

#endif

template<typename T>
class Vector2
{
public:
	Vector2<T>(const Vector2<T>& v) { this->x = v.x; this->y = v.y; }

	Vector2<T>(const T& x, const T& y) : x(x), y(y) {}
	Vector2<T>(const T& v) : x(v), y(v) {}
	Vector2<T>() : x(0), y(0) {}

	~Vector2<T>() {}

	inline Vector2<T> operator+(const Vector2<T>& v) const { return Vector2<T>(x + v.x, y + v.y); }
	inline Vector2<T> operator-(const Vector2<T>& v) const { return Vector2<T>(x - v.x, y - v.y); }
	inline Vector2<T> operator*(const Vector2<T>& v) const { return Vector2<T>(x * v.x, y * v.y); }
	inline Vector2<T> operator/(const Vector2<T>& v) const { return Vector2<T>(x / v.x, y / v.y); }

	inline Vector2<T> operator+(const T& v) const { return Vector2<T>(x + v, y + v); }
	inline Vector2<T> operator-(const T& v) const { return Vector2<T>(x - v, y - v); }
	inline Vector2<T> operator*(const T& v) const { return Vector2<T>(x * v, y * v); }
	inline Vector2<T> operator/(const T& v) const { return Vector2<T>(x / v, y / v); }

	inline Vector2<T> operator+=(const Vector2<T>& v) { return Vector2<T>(x += v.x, y += v.y); }
	inline Vector2<T> operator-=(const Vector2<T>& v) { return Vector2<T>(x -= v.x, y -= v.y); }
	inline Vector2<T> operator*=(const Vector2<T>& v) { return Vector2<T>(x *= v.x, y *= v.y); }
	inline Vector2<T> operator/=(const Vector2<T>& v) { return Vector2<T>(x /= v.x, y /= v.y); }

	inline Vector2<T> operator+=(const T& v) { return Vector2<T>(x += v, y += v); }
	inline Vector2<T> operator-=(const T& v) { return Vector2<T>(x -= v, y -= v); }
	inline Vector2<T> operator*=(const T& v) { return Vector2<T>(x *= v, y *= v); }
	inline Vector2<T> operator/=(const T& v) { return Vector2<T>(x /= v, y /= v); }

	inline T operator[](const int& index) const
	{
		if (index == 0)
			return x;
		else if (index == 1)
			return y;
		else
			return 0;
	}

	inline bool operator<(const Vector2<T>& v) const { return x < v.x && y < v.x; }
	inline bool operator>(const Vector2<T>& v) const { return x > v.x && y > v.x; }
	inline bool operator<=(const Vector2<T>& v) const { return x <= v.x && y <= v.x; }
	inline bool operator>=(const Vector2<T>& v) const { return x >= v.x && y >= v.x; }
	inline bool operator==(const Vector2<T>& v) const { return x == v.x && y == v.y; }
	inline bool operator!=(const Vector2<T>& v) const { return !(operator==(v)); }

	inline T length() const { return sqrt(this->dot(*this)); }
	inline T lengthSquared() const { return this->dot(*this); }
	inline Vector2<T> normalise() const { return *this / length(); }
	inline T dot(const Vector2<T>& v) const { return x * v.x + y * v.y;	}

	inline Vector2<T> abs() const;

	Vector2<T> lerp(const Vector2<T>& b, const T& t, const bool& invert) const;
	Vector2<T> lerp(const Vector2<T>& b, const Vector2<T>& t, const bool& invert) const;

	Vector2<T> swap() const;

	Vector2<T> negate() const { return Vector2<T>(-x, -y); };
	Vector2<T> negated() { this->x = -x; this->y = -y; return *this; };

	Vector2<T>& xx() { return this(x, x); }
	Vector2<T>& yy() { return this(y, y); }

	Vector2<T>& xy() { return this(x, y); }
	Vector2<T>& yx() { return this(y, x); }

	void print() const { PR_LOG_MESSAGE("[ %f, %f ]\n", (T) x, (T) y); }

	static size_t size() { return 2 * sizeof(float); }
public:
	union
	{
		struct { T x, y; };
		struct { T r, g; };
		struct { T s, t; };
	};
};

template<typename T>
class Vector3
{
public:
	Vector3<T>(const Vector3<T>& v) { this->x = v.x; this->y = v.y; this->z = v.z; }

	Vector3<T>(const Vector2<T>& xy, const T& z) : x(xy.x), y(xy.y), z(z) {}
	Vector3<T>(const T& x, const Vector2<T>& yz) : x(x), y(yz.x), z(yz.y) {}
	Vector3<T>(const T& x, T y, T z) : x(x), y(y), z(z) {}
	Vector3<T>(const T& v) : x(v), y(v), z(v) {}
	Vector3<T>() : x(0), y(0), z(0) {}

	~Vector3<T>() {}

	inline Vector3<T> operator+(const Vector3<T>& v) const { return Vector3<T>(x + v.x, y + v.y, z + v.z); }
	inline Vector3<T> operator-(const Vector3<T>& v) const { return Vector3<T>(x - v.x, y - v.y, z - v.z); }
	inline Vector3<T> operator*(const Vector3<T>& v) const { return Vector3<T>(x * v.x, y * v.y, z * v.z); }
	inline Vector3<T> operator/(const Vector3<T>& v) const { return Vector3<T>(x / v.x, y / v.y, z / v.z); }

	inline Vector3<T> operator+(const T& v) const { return Vector3<T>(x + v, y + v, z + v); }
	inline Vector3<T> operator-(const T& v) const { return Vector3<T>(x - v, y - v, z - v); }
	inline Vector3<T> operator*(const T& v) const { return Vector3<T>(x * v, y * v, z * v); }
	inline Vector3<T> operator/(const T& v) const { return Vector3<T>(x / v, y / v, z / v); }

	inline Vector3<T> operator+=(const Vector3<T>& v) { return Vector3<T>(x += v.x, y += v.y, z += v.z); }
	inline Vector3<T> operator-=(const Vector3<T>& v) { return Vector3<T>(x -= v.x, y -= v.y, z -= v.z); }
	inline Vector3<T> operator*=(const Vector3<T>& v) { return Vector3<T>(x *= v.x, y *= v.y, z *= v.z); }
	inline Vector3<T> operator/=(const Vector3<T>& v) { return Vector3<T>(x /= v.x, y /= v.y, z /= v.z); }
	
	inline Vector3<T> operator+=(const T& v) { return Vector3<T>(x += v, y += v, z += v); }
	inline Vector3<T> operator-=(const T& v) { return Vector3<T>(x -= v, y -= v, z -= v); }
	inline Vector3<T> operator*=(const T& v) { return Vector3<T>(x *= v, y *= v, z *= v); }
	inline Vector3<T> operator/=(const T& v) { return Vector3<T>(x /= v, y /= v, z /= v); }
	
	inline T operator[](const int& index) const
	{
		if (index == 0)
			return x;
		else if (index == 1)
			return y;
		else if (index == 2)
			return z;
		else
			return 0;
	}

	inline bool operator<(const Vector3<T>& v) const { return x < v.x && y < v.y && z < v.z; }
	inline bool operator>(const Vector3<T>& v) const { return x > v.x && y > v.y && z > v.z; }
	inline bool operator<=(const Vector3<T>& v) const { return x <= v.x && y <= v.y && z <= v.z; }
	inline bool operator>=(const Vector3<T>& v) const { return x >= v.x && y >= v.y && z >= v.z; }
	inline bool operator==(const Vector3<T>& v) const { return x == v.x && y == v.y && z == v.z; }
	inline bool operator!=(const Vector3<T>& v) const { return !(operator==(v)); }

	inline T length() const { return sqrt(this->dot(*this)); }
	inline T lengthSquared() const { return this->dot(*this); }
	inline Vector3<T> normalise() const { return *this / length(); }
	inline T dot(const Vector3<T>& v) const { return x * v.x + y * v.y + z * v.z; }

	inline Vector3<T> abs() const;

	Vector3<T> cross(const Vector3<T>& normal) const;
	Vector3<T> reflect(const Vector3<T>& normal) const;
	Vector3<T> refract(const Vector3<T>& normal, const T& eta) const;

	Vector3<T> lerp(const Vector3<T>& b, const T& t) const;
	Vector3<T> lerp(const Vector3<T>& b, const Vector3<T>& t) const;

	Vector3<T> rotate(const Vector3<T>& angles) const;
	Vector3<T> rotate(const Vector3<T>& axis, const T& angle) const;

	Vector3<T> negate() const { return Vector3<T>(-x, -y, -z); };
	Vector3<T> negated() { this->x = -x; this->y = -y; this->z = -z; return *this; };

	Vector3<T>& xzy() { return this(x, z, y); }
	Vector3<T>& yxz() { return this(y, x, z); }
	Vector3<T>& yzx() { return this(y, x, y); }
	Vector3<T>& zxy() { return this(z, x, y); }
	Vector3<T>& zyx() { return this(z, y, x); }

	Vector3<T>& xxy() { return this(x, x, y); }
	Vector3<T>& xyx() { return this(x, y, x); }
	Vector3<T>& yxx() { return this(y, x, x); }
	Vector3<T>& xyy() { return this(x, y, y); }
	Vector3<T>& yxy() { return this(y, x, y); }
	Vector3<T>& yyx() { return this(y, y, x); }

	Vector3<T>& xxz() { return this(x, x, z); }
	Vector3<T>& xzx() { return this(x, z, x); }
	Vector3<T>& zxx() { return this(z, x, x); }
	Vector3<T>& xzz() { return this(x, z, z); }
	Vector3<T>& zxz() { return this(z, x, z); }
	Vector3<T>& zzx() { return this(z, z, x); }

	Vector2<T> xy() { return Vector2<T>(x, y); }

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f ]\n", (T) x, (T) y, (T) z); }

	static size_t size() { return 3 * sizeof(float); }
public:
	union
	{
		struct { T x, y, z; };
		struct { T r, g, b; };
		struct { T s, t, r; };
	};
};

template<typename T> class Quaternion;

template<typename T>
class Vector4
{
public:
	Vector4<T>(const Vector4<T>& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }
	Vector4<T>(const Quaternion<T>& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }

	Vector4<T>(const T* v)
	{
		if (&(v[3]) == nullptr)
			PR_LOG_ERROR("You try to create a Vector4<T> object from an array of T with less then 4 members. The array's memory address: %i\n", v);

		this->x = v[0];
		this->y = v[1];
		this->z = v[2];
		this->w = v[3];
	}

	Vector4<T>(const Vector3<T>& xyz, const T& w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	Vector4<T>(const T& x, const Vector3<T>& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
	Vector4<T>(const Vector2<T>& xy, const T& z, const T& w) : x(xy.x), y(xy.y), z(w), w(w) {}
	Vector4<T>(const Vector2<T>& xy, const Vector2<T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	Vector4<T>(const T& x, const Vector2<T>& yz, const T& w) : x(x), y(yz.x), z(yz.y), w(w) {}
	Vector4<T>(const T& x, const T& y, const Vector2<T>& zw) : x(x), y(y), z(zw.x), w(zw.y) {}
	Vector4<T>(const T& x, const T& y, const T& z, const T& w) : x(x), y(y), z(z), w(w) {}
	Vector4<T>(const T& v) : x(v), y(v), z(v), w(v) {}
	Vector4<T>() : x(0), y(0), z(0), w(0) {}

	~Vector4<T>() {}

	inline Vector4<T> operator+(const Vector4<T>& v) const { return Vector4<T>(x + v.x, y + v.y, z + v.z, w + v.w); }
	inline Vector4<T> operator-(const Vector4<T>& v) const { return Vector4<T>(x - v.x, y - v.y, z - v.z, w - v.w); }
	inline Vector4<T> operator*(const Vector4<T>& v) const { return Vector4<T>(x * v.x, y * v.y, z * v.z, w * v.w); }
	inline Vector4<T> operator/(const Vector4<T>& v) const { return Vector4<T>(x / v.x, y / v.y, z / v.z, w / v.w); }

	inline Vector4<T> operator+(const T& v) const { return Vector4<T>(x + v, y + v, z + v, w + v); }
	inline Vector4<T> operator-(const T& v) const { return Vector4<T>(x - v, y - v, z - v, w - v); }
	inline Vector4<T> operator*(const T& v) const { return Vector4<T>(x * v, y * v, z * v, w * v); }
	inline Vector4<T> operator/(const T& v) const { return Vector4<T>(x / v, y / v, z / v, w / v); }

	inline Vector4<T> operator+=(const Vector4<T>& v) { return Vector4<T>(x += v.x, y += v.y, z += v.z, w += v.w); }
	inline Vector4<T> operator-=(const Vector4<T>& v) { return Vector4<T>(x -= v.x, y -= v.y, z -= v.z, w -= v.w); }
	inline Vector4<T> operator*=(const Vector4<T>& v) { return Vector4<T>(x *= v.x, y *= v.y, z *= v.z, w *= v.w); }
	inline Vector4<T> operator/=(const Vector4<T>& v) { return Vector4<T>(x /= v.x, y /= v.y, z /= v.z, w /= v.w); }

	inline Vector4<T> operator+=(const T& v) { return Vector4<T>(x += v, y += v, z += v, w += v); }
	inline Vector4<T> operator-=(const T& v) { return Vector4<T>(x -= v, y -= v, z -= v, w -= v); }
	inline Vector4<T> operator*=(const T& v) { return Vector4<T>(x *= v, y *= v, z *= v, w *= v); }
	inline Vector4<T> operator/=(const T& v) { return Vector4<T>(x /= v, y /= v, z /= v, w /= v); }

	inline T operator[](const int& index) const
	{
		if (index == 0)
			return x;
		else if (index == 1)
			return y;
		else if (index == 2)
			return z;
		else if (index == 3)
			return w;
		else
			return 0;
	}

	inline bool operator<(const Vector4<T>& v) const { return x < v.x && y < v.y && z < v.z && w < v.w; }
	inline bool operator>(const Vector4<T>& v) const { return x > v.x && y > v.y && z > v.z && w > v.w; }
	inline bool operator<=(const Vector4<T>& v) const { return x <= v.x && y <= v.y && z <= v.z && w <= v.w; }
	inline bool operator>=(const Vector4<T>& v) const { return x >= v.x && y >= v.y && z >= v.z && w >= v.w; }
	inline bool operator==(const Vector4<T>& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
	inline bool operator!=(const Vector4<T>& v) const { return !(operator==(v)); }

	inline T length() const { return sqrt(this->dot(*this)); }
	inline T lengthSquared() const { return this->dot(*this); }
	inline Vector4<T> normalise() const { return *this / length(); }
	inline T dot(const Vector4<T>& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

	inline Vector4<T> abs() const;

	Vector4<T> lerp(const Vector4<T>& b, const T& t) const;
	Vector4<T> lerp(const Vector4<T>& b, const Vector4<T>& t) const;

	Vector4<T> negate() const { return Vector4<T>(-x, -y, -z, -w); };
	Vector4<T> negated() { this->x = -x; this->y = -y; this->z = -z; this->w = -w; return *this; };

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f, %f ]\n", (T) x, (T) y, (T) z, (T) w); }

	static size_t size() { return 4 * sizeof(float); }
public:
	union
	{
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};
};

template<typename T>
class Quaternion
{
public:
	Quaternion<T>(const Quaternion<T>& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }
	Quaternion<T>(const Vector4<T>& v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; }

	Quaternion<T>(const T* v)
	{
		if (&(v[3]) == nullptr)
			PR_LOG_ERROR("You try to create a Quaternion<T> object from an array of T with less then 4 members. The array's memory address: %i\n", v);

		this->x = v[0];
		this->y = v[1];
		this->z = v[2];
		this->w = v[3];
	}

	Quaternion<T>(const Vector3<T>& xyz, const T& w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	Quaternion<T>(const T& x, const Vector3<T>& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
	Quaternion<T>(const Vector2<T>& xy, const T& z, const T& w) : x(xy.x), y(xy.y), z(w), w(w) {}
	Quaternion<T>(const Vector2<T>& xy, const Vector2<T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	Quaternion<T>(const T& x, const Vector2<T>& yz, const T& w) : x(x), y(yz.x), z(yz.y), w(w) {}
	Quaternion<T>(const T& x, const T& y, const Vector2<T>& zw) : x(x), y(y), z(zw.x), w(zw.y) {}
	Quaternion<T>(const T& x, const T& y, const T& z, const T& w) : x(x), y(y), z(z), w(w) {}
	Quaternion<T>(const T& v) : x(v), y(v), z(v), w(v) {}
	Quaternion<T>() : x(0), y(0), z(0), w(0) {}

	~Quaternion<T>() {}

	inline Quaternion<T> operator+(const Vector4<T>& v) const { return Quaternion<T>(x + v.x, y + v.y, z + v.z, w + v.w); }
	inline Quaternion<T> operator-(const Vector4<T>& v) const { return Quaternion<T>(x - v.x, y - v.y, z - v.z, w - v.w); }
	inline Quaternion<T> operator+(const Quaternion<T>& v) const { return Quaternion<T>(x + v.x, y + v.y, z + v.z, w + v.w); }
	inline Quaternion<T> operator-(const Quaternion<T>& v) const { return Quaternion<T>(x - v.x, y - v.y, z - v.z, w - v.w); }

	inline Quaternion<T> operator+(const T& v) const { return Quaternion<T>(x + v, y + v, z + v, w + v); }
	inline Quaternion<T> operator-(const T& v) const { return Quaternion<T>(x - v, y - v, z - v, w - v); }
	inline Quaternion<T> operator*(const T& v) const { return Quaternion<T>(x * v, y * v, z * v, w * v); }
	inline Quaternion<T> operator/(const T& v) const { return Quaternion<T>(x / v, y / v, z / v, w / v); }

	inline Quaternion<T> operator+=(const Vector4<T>& v) { return Quaternion<T>(x += v.x, y += v.y, z += v.z, w += v.w); }
	inline Quaternion<T> operator-=(const Vector4<T>& v) { return Quaternion<T>(x -= v.x, y -= v.y, z -= v.z, w -= v.w); }
	inline Quaternion<T> operator+=(const Quaternion<T>& v) { return Quaternion<T>(x += v.x, y += v.y, z += v.z, w += v.w); }
	inline Quaternion<T> operator-=(const Quaternion<T>& v) { return Quaternion<T>(x -= v.x, y -= v.y, z -= v.z, w -= v.w); }

	inline Quaternion<T> operator+=(const T& v) { return Quaternion<T>(x += v, y += v, z += v, w += v); }
	inline Quaternion<T> operator-=(const T& v) { return Quaternion<T>(x -= v, y -= v, z -= v, w -= v); }
	inline Quaternion<T> operator*=(const T& v) { return Quaternion<T>(x *= v, y *= v, z *= v, w *= v); }
	inline Quaternion<T> operator/=(const T& v) { return Quaternion<T>(x /= v, y /= v, z /= v, w /= v); }

	inline Quaternion<T> Conjugate() const { return Quaternion<T>(-x, -y, -z, w); }

	Quaternion<T> operator*(const Quaternion<T>& v) const;
	Quaternion<T> operator*(const Vector4<T>& v) const;
	Quaternion<T> operator*(const Vector3<T>& v) const;

	Quaternion<T> operator*=(const Quaternion<T>& v) const;
	Quaternion<T> operator*=(const Vector4<T>& v) const;
	Quaternion<T> operator*=(const Vector3<T>& v) const;

	inline T operator[](const int& index) const
	{
		if (index == 0)
			return x;
		else if (index == 1)
			return y;
		else if (index == 2)
			return z;
		else if (index == 3)
			return w;
		else
			return 0;
	}

	inline bool operator<(const Quaternion<T>& v) const { return x < v.x&& y < v.x&& z < v.z&& w < v.w; }
	inline bool operator>(const Quaternion<T>& v) const { return x > v.x && y > v.x && z > v.z && w > v.w; }
	inline bool operator<=(const Quaternion<T>& v) const { return x <= v.x && y <= v.x && z <= v.z && w <= v.w; }
	inline bool operator>=(const Quaternion<T>& v) const { return x >= v.x && y >= v.x && z >= v.z && w >= v.w; }
	inline bool operator==(const Quaternion<T>& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
	inline bool operator!=(const Quaternion<T>& v) const { return !(operator==(v)); }

	inline T length() const { return sqrt(this->dot(*this)); }
	inline T lengthSquared() const { return this->dot(*this); }
	inline Quaternion<T> normalise() const { return *this / length(); }
	inline T dot(const Quaternion<T>& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

	inline Quaternion<T> abs() const;

	Quaternion<T> lerp(const Quaternion<T>& b, const T& t) const;
	Quaternion<T> lerp(const Quaternion<T>& b, const Quaternion<T>& t) const;

	Quaternion<T> negate() const { return Quaternion<T>(-x, -y, -z, -w); };
	Quaternion<T> negated() { this->x = -x; this->y = -y; this->z = -z; this->w = -w; return *this; };

	void print() const { PR_LOG_MESSAGE("[ %f, %f, %f, %f ]", (T) x, (T) y, (T) z, (T) w); }
public:
	union
	{
		struct { T i, j, k, l; };
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};
};

typedef Vector2<int> Vector2i;
typedef Vector3<int> Vector3i;
typedef Vector4<int> Vector4fi;
typedef Quaternion<int> Quaternioni;

typedef Vector2<uint32_t> Vector2u;
typedef Vector3<uint32_t> Vector3u;
typedef Vector4<uint32_t> Vector4fu;
typedef Quaternion<uint32_t> Quaternionu;

#if !defined(PR_WINDOWS_64) || !defined(PR_FAST_MATH)
typedef Vector2<float> Vector2f;
typedef Vector3<float> Vector3f;
typedef Vector4<float> Vector4f;
typedef Quaternion<float> Quaternionf;
#endif

typedef Vector2<double> Vector2d;
typedef Vector3<double> Vector3d;
typedef Vector4<double> Vector4fd;
typedef Quaternion<double> Quaterniond;

#if !defined(PR_WINDOWS_64) || !defined(PR_FAST_MATH)
class Matrix4f
{
public:
	Matrix4f(const Matrix4f& v);
	Matrix4f(Matrix4f&& v) noexcept;

	Matrix4f(const Vector4f& v);
	Matrix4f();

	~Matrix4f();

	Matrix4f& operator=(const Matrix4f& v);
	Matrix4f& operator=(Matrix4f&& v) noexcept;

	inline Matrix4f operator+(const Matrix4f& v) const;
	inline Matrix4f operator-(const Matrix4f& v) const;

	inline Matrix4f operator+=(const Matrix4f& v);
	inline Matrix4f operator-=(const Matrix4f& v);

	inline Matrix4f operator+(const Vector4f& v) const;
	inline Matrix4f operator-(const Vector4f& v) const;

	inline Matrix4f operator+=(const Vector4f& v);
	inline Matrix4f operator-=(const Vector4f& v);

	inline Matrix4f operator+(const float& v) const;
	inline Matrix4f operator-(const float& v) const;

	inline Matrix4f operator+=(const float& v);
	inline Matrix4f operator-=(const float& v);

	inline Matrix4f operator*(const Matrix4f& v) const;
	inline Matrix4f operator*=(const Matrix4f& v);

	Vector4f operator*(const Vector4f& v) const;

	inline bool operator==(const Matrix4f& v) const;

	std::array<float, 4> operator[](const int& index) const;

	static Matrix4f Identity();
	static Matrix4f Zero();

	static Matrix4f Translation(const Vector3f& position);
	static Matrix4f Rotation(const Vector3f& rotation);
	static Matrix4f Scaling(const Vector3f& scale);

	static Matrix4f Transformation(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scaling);
	static Matrix4f PerspectiveProjection(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane);
	static Matrix4f View(const Vector3f& forward, const Vector3f& up);

	std::array<float, 4> const getRow(const int i) const;

	void print() const
	{
		PR_LOG_MESSAGE("Matrix: \n");
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				PR_LOG_MESSAGE("\t%f", m[j * 4 + i]);
			}

			PR_LOG_MESSAGE("\n");
		}
	}
private:
	void clear();
public:
	float* m;
};
#endif

#endif