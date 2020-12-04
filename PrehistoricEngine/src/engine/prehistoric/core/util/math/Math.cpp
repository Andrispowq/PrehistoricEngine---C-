#include "engine/prehistoric/core/util/Includes.hpp"
#include "Math.h"

#if defined(PR_FAST_MATH)

inline Vector2f Vector2f::abs() const
{
	return Vector2f(fabs((*this).x), fabs((*this).y));
}

Vector2f Vector2f::lerp(const Vector2f& b, const float& t, const bool& invert) const
{
	float x_ = x + (b.x - x) * t;
	float y_ = y + (b.y - y) * t;

	return invert ? Vector2f(y_, x_) : Vector2f(x_, y_);
}

Vector2f Vector2f::lerp(const Vector2f& b, const Vector2f& t, const bool& invert) const
{
	float x_ = x + (b.x - x) * t.x;
	float y_ = y + (b.y - y) * t.y;

	return invert ? Vector2f(y_, x_) : Vector2f(x_, y_);
}

Vector2f Vector2f::swap() const
{
	return Vector2f(y, x);
}

inline Vector3f Vector3f::abs() const
{
	return Vector3f(std::abs(x), std::abs(y), std::abs(z));
}

Vector3f Vector3f::cross(const Vector3f& v) const
{
	float x_ = y * v.z - z * v.y;
	float y_ = z * v.x - x * v.z;
	float z_ = x * v.y - y * v.x;

	return Vector3f(x_, y_, z_);
}

Vector3f Vector3f::reflect(const Vector3f& normal) const
{
	return *this - normal * 2 * this->dot(normal);
}

Vector3f Vector3f::refract(const Vector3f& normal, const float& eta) const
{
	float k = 1 - std::pow(eta, 2) * (1 - std::pow(this->dot(normal), 2));

	if (k < 0)
		return Vector3f(0);
	else
		return *this * eta - normal * (eta * this->dot(normal) + std::sqrt(k));
}

Vector3f Vector3f ::lerp(const Vector3f& b, const float& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), _mm_set_ss(t)), reg);
}

Vector3f Vector3f::lerp(const Vector3f& b, const Vector3f& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), t.reg), reg);
}

Vector3f Vector3f::rotate(const Vector3f& angles) const
{
	/*Vector3f rotX = this->rotate(Vector3f(1, 0, 0), angles.x);
	Vector3f rotY = this->rotate(Vector3f(0, 1, 0), angles.y);
	Vector3f rotZ = this->rotate(Vector3f(0, 0, 1), angles.z);

	return rotX + rotY + rotZ - *this * 2;*/

	Matrix4f rot = Matrix4f::Rotation(angles);
	return (rot * Vector4f(*this, 0)).xyz();
}

Vector3f Vector3f::rotate(const Vector3f& axis, const float& angle) const
{
	Vector3f result;

	float sinHalfAngle = std::sin(ToRadians(angle / 2));
	float cosHalfAngle = std::cos(ToRadians(angle / 2));

	float rx = axis.x * sinHalfAngle;
	float ry = axis.y * sinHalfAngle;
	float rz = axis.z * sinHalfAngle;
	float rw = cosHalfAngle;

	Quaternionf rotation(rx, ry, rz, rw);
	Quaternionf conjugate = rotation.Conjugate();

	Quaternionf w = rotation * (*this) * conjugate;

	result.x = w.x;
	result.y = w.y;
	result.z = w.z;

	return result;
}

inline Vector4f Vector4f::abs() const
{
	return Vector4f(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

Vector4f Vector4f::lerp(const Vector4f& b, const float& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), _mm_set_ss(t)), reg);
}

Vector4f Vector4f::lerp(const Vector4f& b, const Vector4f& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), t.reg), reg);
}

Quaternionf Quaternionf::operator*(const Quaternionf& r) const
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	return Quaternionf(x_, y_, z_, w_);
}

Quaternionf Quaternionf::operator*(const Vector4f& r) const
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	return Quaternionf(x_, y_, z_, w_);
}

Quaternionf Quaternionf::operator*(const Vector3f& r) const
{
	float x_ = w * r.x + y * r.z - z * r.y;
	float y_ = w * r.y + z * r.x - x * r.z;
	float z_ = w * r.z + x * r.y - y * r.x;
	float w_ = -x * r.x - y * r.y - z * r.z;

	return Quaternionf(x_, y_, z_, w_);
}

Quaternionf Quaternionf::operator*=(const Quaternionf& r)
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	this->x = x_;
	this->y = y_;
	this->z = z_;
	this->w = w_;

	return *this;
}

Quaternionf Quaternionf::operator*=(const Vector4f& r)
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	this->x = x_;
	this->y = y_;
	this->z = z_;
	this->w = w_;

	return *this;
}

Quaternionf Quaternionf::operator*=(const Vector3f& r)
{
	float x_ = w * r.x + y * r.z - z * r.y;
	float y_ = w * r.y + z * r.x - x * r.z;
	float z_ = w * r.z + x * r.y - y * r.x;
	float w_ = -x * r.x - y * r.y - z * r.z;

	this->x = x_;
	this->y = y_;
	this->z = z_;
	this->w = w_;

	return *this;
}

inline Quaternionf Quaternionf::abs() const
{
	return Quaternionf(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

Quaternionf Quaternionf::lerp(const Quaternionf& b, const float& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), _mm_set_ss(t)), reg);
}

Quaternionf Quaternionf::lerp(const Quaternionf& b, const Quaternionf& t) const
{
	return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b.reg, reg), t.reg), reg);
}

Matrix4f::Matrix4f(const Matrix4f& v)
{
	m = new float[4 * 4];

	memcpy(this->m, v.m, sizeof(float) * 16);
}

Matrix4f::Matrix4f(Matrix4f&& v) noexcept
{
	m = v.m;
	v.m = nullptr;
}

Matrix4f::Matrix4f(const Vector4f& v)
{
	m = new float[4 * 4];

	for (int i = 0; i < 4; i++)
	{
		m[i * 4 + 0] = v.x;
		m[i * 4 + 1] = v.y;
		m[i * 4 + 2] = v.z;
		m[i * 4 + 3] = v.w;
	}
}

Matrix4f::Matrix4f()
{
	m = new float[4 * 4];

	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
}

Matrix4f::~Matrix4f()
{
	clear();
}

Matrix4f& Matrix4f::operator=(const Matrix4f& v)
{
	delete[] m;

	this->m = new float[4 * 4];
	memcpy(this->m, v.m, sizeof(float) * 16);

	return *this;
}

Matrix4f& Matrix4f::operator=(Matrix4f&& v) noexcept
{
	this->m = v.m;
	v.m = nullptr;

	return *this;
}

inline Matrix4f Matrix4f::operator+(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_add_ps(coloumns[i], v.coloumns[i]);
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_sub_ps(coloumns[i], v.coloumns[i]);
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const Matrix4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_add_ps(coloumns[i], v.coloumns[i]);
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const Matrix4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_sub_ps(coloumns[i], v.coloumns[i]);
	}

	return *this;
}

inline Matrix4f Matrix4f::operator+(const Vector4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_add_ps(coloumns[i], v.reg);
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const Vector4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_sub_ps(coloumns[i], v.reg);
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const Vector4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_add_ps(coloumns[i], v.reg);
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const Vector4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_sub_ps(coloumns[i], v.reg);
	}

	return *this;
}

inline Matrix4f Matrix4f::operator+(const float& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_add_ps(coloumns[i], _mm_set_ps(v, v, v, v));
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const float& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.coloumns[i] = _mm_sub_ps(coloumns[i], _mm_set_ps(v, v, v, v));
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const float& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_add_ps(coloumns[i], _mm_set_ps(v, v, v, v));
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const float& v)
{
	for (int i = 0; i < 4; i++)
	{
		coloumns[i] = _mm_sub_ps(coloumns[i], _mm_set_ps(v, v, v, v));
	}

	return *this;
}

inline Matrix4f Matrix4f::operator*(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		__m128 res0 = _mm_mul_ps(_mm_set_ps(m[0 * 4 + 3], m[0 * 4 + 2], m[0 * 4 + 1], m[0 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 0], v.m[i * 4 + 0], v.m[i * 4 + 0], v.m[i * 4 + 0]));
		__m128 res1 = _mm_mul_ps(_mm_set_ps(m[1 * 4 + 3], m[1 * 4 + 2], m[1 * 4 + 1], m[1 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 1], v.m[i * 4 + 1], v.m[i * 4 + 1], v.m[i * 4 + 1]));
		__m128 res2 = _mm_mul_ps(_mm_set_ps(m[2 * 4 + 3], m[2 * 4 + 2], m[2 * 4 + 1], m[2 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 2], v.m[i * 4 + 2], v.m[i * 4 + 2], v.m[i * 4 + 2]));
		__m128 res3 = _mm_mul_ps(_mm_set_ps(m[3 * 4 + 3], m[3 * 4 + 2], m[3 * 4 + 1], m[3 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 3], v.m[i * 4 + 3], v.m[i * 4 + 3], v.m[i * 4 + 3]));

		res.coloumns[i] = _mm_add_ps(res0, res1);
		res.coloumns[i] = _mm_add_ps(res.coloumns[i], res2);
		res.coloumns[i] = _mm_add_ps(res.coloumns[i], res3);
	}

	return res;
}

inline Matrix4f Matrix4f::operator*=(const Matrix4f& v)
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		__m128 res0 = _mm_mul_ps(_mm_set_ps(m[0 * 4 + 3], m[0 * 4 + 2], m[0 * 4 + 1], m[0 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 0], v.m[i * 4 + 0], v.m[i * 4 + 0], v.m[i * 4 + 0]));
		__m128 res1 = _mm_mul_ps(_mm_set_ps(m[1 * 4 + 3], m[1 * 4 + 2], m[1 * 4 + 1], m[1 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 1], v.m[i * 4 + 1], v.m[i * 4 + 1], v.m[i * 4 + 1]));
		__m128 res2 = _mm_mul_ps(_mm_set_ps(m[2 * 4 + 3], m[2 * 4 + 2], m[2 * 4 + 1], m[2 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 2], v.m[i * 4 + 2], v.m[i * 4 + 2], v.m[i * 4 + 2]));
		__m128 res3 = _mm_mul_ps(_mm_set_ps(m[3 * 4 + 3], m[3 * 4 + 2], m[3 * 4 + 1], m[3 * 4 + 0]), _mm_set_ps(v.m[i * 4 + 3], v.m[i * 4 + 3], v.m[i * 4 + 3], v.m[i * 4 + 3]));

		res.coloumns[i] = _mm_add_ps(res0, res1);
		res.coloumns[i] = _mm_add_ps(res.coloumns[i], res2);
		res.coloumns[i] = _mm_add_ps(res.coloumns[i], res3);
	}

	for (int i = 0; i < 4; i++)
	{
		this->coloumns[i] = res.coloumns[i];
	}

	return *this;
}

Vector4f Matrix4f::operator*(const Vector4f& v) const
{
	Vector4f res = Vector4f();

	__m128 res0 = _mm_mul_ps(coloumns[0], _mm_set_ps(v.x, v.x, v.x, v.x));
	__m128 res1 = _mm_mul_ps(coloumns[1], _mm_set_ps(v.y, v.y, v.y, v.y));
	__m128 res2 = _mm_mul_ps(coloumns[2], _mm_set_ps(v.z, v.z, v.z, v.z));
	__m128 res3 = _mm_mul_ps(coloumns[3], _mm_set_ps(v.w, v.w, v.w, v.w));

	res.reg = _mm_add_ps(res0, res1);
	res.reg = _mm_add_ps(res.reg, res2);
	res.reg = _mm_add_ps(res.reg, res3);

	return res;
}

inline bool Matrix4f::operator==(const Matrix4f& v) const
{
	for (int i = 0; i < 4; i++)
	{
		__m128 res = _mm_cmpeq_ps(coloumns[i], v.coloumns[i]);

		if (res.m128_f32[0] != 0 || res.m128_f32[1] != 0 || res.m128_f32[2] != 0 || res.m128_f32[3] != 0)
		{
			return false;
		}
	}

	return true;
}

std::array<float, 4> Matrix4f::operator[](int index) const
{
	std::array<float, 4> arr{};

	if (index > 3)
		return arr;

	for (int i = 0; i < 4; i++)
	{
		arr[i] = m[index * 4 + i];
	}

	return arr;
}

Matrix4f Matrix4f::Identity()
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
				res.m[i * 4 + j] = 1;
			else
				res.m[i * 4 + j] = 0;
		}
	}

	return res;
}

Matrix4f Matrix4f::Zero()
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = 0;
	}

	return res;
}

Matrix4f Matrix4f::Translation(const Vector3f& position)
{
	Matrix4f res = Matrix4f::Identity();

	res.m[3 * 4 + 0] = position.x;
	res.m[3 * 4 + 1] = position.y;
	res.m[3 * 4 + 2] = position.z;

	return res;
}

Matrix4f Matrix4f::Rotation(const Vector3f& rotation)
{
	Matrix4f rx = Matrix4f::Identity();
	Matrix4f ry = Matrix4f::Identity();
	Matrix4f rz = Matrix4f::Identity();

	float x = ToRadians(rotation.x);
	float y = ToRadians(rotation.y);
	float z = ToRadians(rotation.z);

	float cosX = cos(x);
	float cosY = cos(y);
	float cosZ = cos(z);

	float sinX = sin(x);
	float sinY = sin(y);
	float sinZ = sin(z);

	rx.m[1 * 4 + 1] = cosX;
	rx.m[2 * 4 + 1] = -sinX;
	rx.m[1 * 4 + 2] = sinX;
	rx.m[2 * 4 + 2] = cosX;

	ry.m[0 * 4 + 0] = cosY;
	ry.m[2 * 4 + 0] = sinY;
	ry.m[0 * 4 + 2] = -sinY;
	ry.m[2 * 4 + 2] = cosY;

	rz.m[0 * 4 + 0] = cosZ;
	rz.m[1 * 4 + 0] = -sinZ;
	rz.m[0 * 4 + 1] = sinZ;
	rz.m[1 * 4 + 1] = cosZ;

	return ry * rx * rz;
}

Matrix4f Matrix4f::Scaling(const Vector3f& scale)
{
	Matrix4f res = Matrix4f::Identity();

	res.m[0 * 4 + 0] = scale.x;
	res.m[1 * 4 + 1] = scale.y;
	res.m[2 * 4 + 2] = scale.z;

	return res;
}

Matrix4f Matrix4f::Transformation(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scaling)
{
	Matrix4f Translation = Matrix4f::Translation(translation);
	Matrix4f Rotation = Matrix4f::Rotation(rotation);
	Matrix4f Scaling = Matrix4f::Scaling(scaling);

	return Translation * Rotation * Scaling;
}

Matrix4f Matrix4f::PerspectiveProjection(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane)
{
	Matrix4f res = Matrix4f::Identity();

	float tanFOV = tan(ToRadians(fov / 2));
	float frustumLength = farPlane - nearPlane;

	res.m[0 * 4 + 0] = 1 / (tanFOV * aspectRatio);
	res.m[1 * 4 + 1] = 1 / tanFOV;
	res.m[2 * 4 + 2] = (farPlane + nearPlane) / frustumLength;
	res.m[3 * 4 + 2] = -(2 * farPlane * nearPlane) / frustumLength;
	res.m[2 * 4 + 3] = 1;
	res.m[3 * 4 + 3] = 0;

	return res;
}

Matrix4f Matrix4f::View(const Vector3f& forward, const Vector3f& up)
{
	Vector3f right = up.cross(forward);

	Matrix4f mat = Matrix4f::Identity();

	mat.m[0 * 4 + 0] = right.x;
	mat.m[1 * 4 + 0] = right.y;
	mat.m[2 * 4 + 0] = right.z;

	mat.m[0 * 4 + 1] = up.x;
	mat.m[1 * 4 + 1] = up.y;
	mat.m[2 * 4 + 1] = up.z;

	mat.m[0 * 4 + 2] = forward.x;
	mat.m[1 * 4 + 2] = forward.y;
	mat.m[2 * 4 + 2] = forward.z;

	return mat;
}

/*
	The parameter <i> is the index of the row, so to get the first row, the parameter MUST be 0
*/
std::array<float, 4> const Matrix4f::getRow(int i) const
{
	std::array<float, 4> arr;

	for (int j = 0; j < 4; j++)
	{
		arr[j] = m[j * 4 + i];
	}

	return arr;
}

void Matrix4f::clear()
{
	delete[] m;
}

#else

Matrix4f::Matrix4f(const Matrix4f& v)
{
	m = new float[4 * 4];

	memcpy(this->m, v.m, sizeof(float) * 16);
}

Matrix4f::Matrix4f(Matrix4f&& v) noexcept
{
	m = v.m;
	v.m = nullptr;
}

Matrix4f::Matrix4f(const Vector4f& v)
{
	m = new float[4 * 4];

	for (int i = 0; i < 4; i++)
	{
		m[i * 4 + 0] = v.x;
		m[i * 4 + 1] = v.y;
		m[i * 4 + 2] = v.z;
		m[i * 4 + 3] = v.w;
	}
}

Matrix4f::Matrix4f()
{
	m = new float[4 * 4];

	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
}

Matrix4f::~Matrix4f()
{
	clear();
}

Matrix4f& Matrix4f::operator=(const Matrix4f& v)
{
	delete[] m;

	m = new float[4 * 4];
	memcpy(this->m, v.m, sizeof(float) * 16);

	return *this;
}

Matrix4f& Matrix4f::operator=(Matrix4f&& v) noexcept
{
	m = v.m;
	v.m = nullptr;

	return *this;
}

inline Matrix4f Matrix4f::operator+(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = m[i] + v.m[i];
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = m[i] - v.m[i];
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const Matrix4f& v)
{
	for (int i = 0; i < 16; i++)
	{
		this->m[i] = m[i] + v.m[i];
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const Matrix4f& v)
{
	for (int i = 0; i < 16; i++)
	{
		this->m[i] = m[i] - v.m[i];
	}

	return *this;
}

inline Matrix4f Matrix4f::operator+(const Vector4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.m[i * 4 + 0] = m[i * 4 + 0] + v.x; 
		res.m[i * 4 + 1] = m[i * 4 + 1] + v.y;
		res.m[i * 4 + 2] = m[i * 4 + 2] + v.z;
		res.m[i * 4 + 3] = m[i * 4 + 3] + v.w;
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const Vector4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		res.m[i * 4 + 0] = m[i * 4 + 0] - v.x;
		res.m[i * 4 + 1] = m[i * 4 + 1] - v.y;
		res.m[i * 4 + 2] = m[i * 4 + 2] - v.z;
		res.m[i * 4 + 3] = m[i * 4 + 3] - v.w;
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const Vector4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		m[i * 4 + 0] += v.x;
		m[i * 4 + 1] += v.y;
		m[i * 4 + 2] += v.z;
		m[i * 4 + 3] += v.w;
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const Vector4f& v)
{
	for (int i = 0; i < 4; i++)
	{
		m[i * 4 + 0] -= v.x;
		m[i * 4 + 1] -= v.y;
		m[i * 4 + 2] -= v.z;
		m[i * 4 + 3] -= v.w;
	}

	return *this;
}

inline Matrix4f Matrix4f::operator+(const float& v) const
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = m[i] + v;
	}

	return res;
}

inline Matrix4f Matrix4f::operator-(const float& v) const
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = m[i] - v;
	}

	return res;
}

inline Matrix4f Matrix4f::operator+=(const float& v)
{
	for (int i = 0; i < 16; i++)
	{
		this->m[i] += v;
	}

	return *this;
}

inline Matrix4f Matrix4f::operator-=(const float& v)
{
	for (int i = 0; i < 16; i++)
	{
		this->m[i] -= v;
	}

	return *this;
}

inline Matrix4f Matrix4f::operator*(const Matrix4f& v) const
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			//j -> this matrix
			//i -> v matrix
			res.m[i * 4 + j] = v.m[i * 4 + 0] * m[0 * 4 + j] + v.m[i * 4 + 1] * m[1 * 4 + j] + v.m[i * 4 + 2] * m[2 * 4 + j] + v.m[i * 4 + 3] * m[3 * 4 + j];
		}
	}

	return res;
}

inline Matrix4f Matrix4f::operator*=(const Matrix4f& v)
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			res.m[i * 4 + j] = v.m[i * 4 + 0] * m[0 * 4 + j] + v.m[i * 4 + 1] * m[1 * 4 + j] + v.m[i * 4 + 2] * m[2 * 4 + j] + v.m[i * 4 + 3] * m[3 * 4 + j];
		}
	}

	for (int i = 0; i < 16; i++)
	{
		this->m[i] = res.m[i];
	}

	return *this;
}

Vector4f Matrix4f::operator*(const Vector4f& v) const
{
	Vector4f res = Vector4f();

	res.x = m[0 * 4 + 0] * v.x + m[1 * 4 + 0] * v.y + m[2 * 4 + 0] * v.z + m[3 * 4 + 0] * v.w;
	res.y = m[0 * 4 + 1] * v.x + m[1 * 4 + 1] * v.y + m[2 * 4 + 1] * v.z + m[3 * 4 + 1] * v.w;
	res.z = m[0 * 4 + 2] * v.x + m[1 * 4 + 2] * v.y + m[2 * 4 + 2] * v.z + m[3 * 4 + 2] * v.w;
	res.w = m[0 * 4 + 3] * v.x + m[1 * 4 + 3] * v.y + m[2 * 4 + 3] * v.z + m[3 * 4 + 3] * v.w;

	return res;
}

inline bool Matrix4f::operator==(const Matrix4f& v) const
{
	for (int i = 0; i < 16; i++)
	{
		if (this->m[i] != v.m[i])
		{
			return false;
		}
	}

	return true;
}

std::array<float, 4> Matrix4f::operator[](int index) const
{
	std::array<float, 4> arr;

	if (index > 3)
		return arr;

	for (int i = 0; i < 4; i++)
	{
		arr[i] = m[index * 4 + i];
	}

	return arr;
}

Matrix4f Matrix4f::Identity()
{
	Matrix4f res;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
				res.m[i * 4 + j] = 1;
			else
				res.m[i * 4 + j] = 0;
		}
	}

	return res;
}

Matrix4f Matrix4f::Zero()
{
	Matrix4f res;

	for (int i = 0; i < 16; i++)
	{
		res.m[i] = 0;
	}

	return res;
}

Matrix4f Matrix4f::Translation(const Vector3f& position)
{
	Matrix4f res = Matrix4f::Identity();

	res.m[3 * 4 + 0] = position.x;
	res.m[3 * 4 + 1] = position.y;
	res.m[3 * 4 + 2] = position.z;

	return res;
}

Matrix4f Matrix4f::Rotation(const Vector3f& rotation)
{
	Matrix4f rx = Matrix4f::Identity();
	Matrix4f ry = Matrix4f::Identity();
	Matrix4f rz = Matrix4f::Identity();

	float x = ToRadians(rotation.x);
	float y = ToRadians(rotation.y);
	float z = ToRadians(rotation.z);

	float cosX = cos(x);
	float cosY = cos(y);
	float cosZ = cos(z);

	float sinX = sin(x);
	float sinY = sin(y);
	float sinZ = sin(z);

	rx.m[1 * 4 + 1] = cosX;
	rx.m[2 * 4 + 1] = -sinX;
	rx.m[1 * 4 + 2] = sinX;
	rx.m[2 * 4 + 2] = cosX;

	ry.m[0 * 4 + 0] = cosY;
	ry.m[2 * 4 + 0] = sinY;
	ry.m[0 * 4 + 2] = -sinY;
	ry.m[2 * 4 + 2] = cosY;

	rz.m[0 * 4 + 0] = cosZ;
	rz.m[1 * 4 + 0] = -sinZ;
	rz.m[0 * 4 + 1] = sinZ;
	rz.m[1 * 4 + 1] = cosZ;

	return ry * rx * rz;
}

Matrix4f Matrix4f::Scaling(const Vector3f& scale)
{
	Matrix4f res = Matrix4f::Identity();

	res.m[0 * 4 + 0] = scale.x;
	res.m[1 * 4 + 1] = scale.y;
	res.m[2 * 4 + 2] = scale.z;

	return res;
}

Matrix4f Matrix4f::Transformation(const Vector3f& translation, const Vector3f& rotation, const Vector3f& scaling)
{
	Matrix4f Translation = Matrix4f::Translation(translation);
	Matrix4f Rotation = Matrix4f::Rotation(rotation);
	Matrix4f Scaling = Matrix4f::Scaling(scaling);

	return Translation * Rotation * Scaling;
}

Matrix4f Matrix4f::PerspectiveProjection(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane)
{
	Matrix4f res = Matrix4f::Identity();

	float tanFOV = tan(ToRadians(fov / 2));
	float frustumLength = farPlane - nearPlane;

	res.m[0 * 4 + 0] = 1 / (tanFOV * aspectRatio);
	res.m[1 * 4 + 1] = 1 / tanFOV;
	res.m[2 * 4 + 2] = (farPlane + nearPlane) / frustumLength;
	res.m[3 * 4 + 2] = -(2 * farPlane * nearPlane) / frustumLength;
	res.m[2 * 4 + 3] = 1;
	res.m[3 * 4 + 3] = 0;

	return res;
}

Matrix4f Matrix4f::View(const Vector3f& forward, const Vector3f& up)
{
	Vector3f right = up.cross(forward);

	Matrix4f mat = Matrix4f::Identity();

	mat.m[0 * 4 + 0] = right.x;
	mat.m[1 * 4 + 0] = right.y;
	mat.m[2 * 4 + 0] = right.z;

	mat.m[0 * 4 + 1] = up.x;
	mat.m[1 * 4 + 1] = up.y;
	mat.m[2 * 4 + 1] = up.z;

	mat.m[0 * 4 + 2] = forward.x;
	mat.m[1 * 4 + 2] = forward.y;
	mat.m[2 * 4 + 2] = forward.z;

	return mat;
}

/*
	The parameter <i> is the index of the row, so to get the first row, the parameter MUST be 0
*/
std::array<float, 4> const Matrix4f::getRow(int i) const
{
	std::array<float, 4> arr;

	for (int j = 0; j < 4; j++)
	{
		arr[j] = m[j * 4 + i];
	}

	return arr;
}

void Matrix4f::clear()
{
	delete[] m;
}

#endif