#if !defined(PR_FAST_MATH)

template<typename T>
inline Vector2<T> Vector2<T>::abs() const
{
	return Vector2(fabs((*this).x), fabs((*this).y));
}

template<typename T>
Vector2<T> Vector2<T>::lerp(const Vector2<T>& b, const T& t, const bool& invert) const
{
	float x_ = x + (b.x - x) * t;
	float y_ = y + (b.y - y) * t;

	return invert ? Vector2<T>(y_, x_) : Vector2<T>(x_, y_);
}

template<typename T>
Vector2<T> Vector2<T>::lerp(const Vector2<T>& b, const Vector2<T>& t, const bool& invert) const
{
	float x_ = x + (b.x - x) * t.x;
	float y_ = y + (b.y - y) * t.y;

	return invert ? Vector2<T>(y_, x_) : Vector2<T>(x_, y_);
}

template<typename T>
Vector2<T> Vector2<T>::swap() const
{
	return Vector2<T>(y, x);
}

template<typename T>
inline Vector3<T> Vector3<T>::abs() const
{
	return Vector3<T>(std::abs(x), std::abs(y), std::abs(z));
}

template<typename T>
Vector3<T> Vector3<T>::cross(const Vector3<T>& v) const
{
	float x_ = y * v.z - z * v.y;
	float y_ = z * v.x - x * v.z;
	float z_ = x * v.y - y * v.x;

	return Vector3<T>(x_, y_, z_);
}

template<typename T>
Vector3<T> Vector3<T>::reflect(const Vector3<T>& normal) const
{
	return *this - normal * 2 * this->dot(normal);
}

template<typename T>
Vector3<T> Vector3<T>::refract(const Vector3<T>& normal, const T& eta) const
{
	float k = 1 - std::pow(eta, 2) * (1 - std::pow(this->dot(normal), 2));

	if (k < 0)
		return Vector3<T>(0);
	else
		return *this * eta - normal * (eta * this->dot(normal) + std::sqrt(k));
}

template<typename T>
Vector3<T> Vector3<T>::lerp(const Vector3<T>& b, const T& t) const
{
	float x_ = x + (b.x - x) * t;
	float y_ = y + (b.y - y) * t;
	float z_ = z + (b.z - z) * t;

	return Vector3<T>(x_, y_, z_);
}

template<typename T>
Vector3<T> Vector3<T>::lerp(const Vector3<T>& b, const Vector3<T>& t) const
{
	float x_ = x + (b.x - x) * t.x;
	float y_ = y + (b.y - y) * t.y;
	float z_ = z + (b.z - z) * t.z;

	return Vector3<T>(x_, y_, z_);
}

template<typename T>
Vector3<T> Vector3<T>::rotate(const Vector3<T>& angles) const
{
	Vector3<T> rotX = this->rotate(Vector3<T>(1, 0, 0), angles.x);
	Vector3<T> rotY = this->rotate(Vector3<T>(0, 1, 0), angles.y);
	Vector3<T> rotZ = this->rotate(Vector3<T>(0, 0, 1), angles.z);

	return rotX + rotY + rotZ - *this * 2;
}

template<typename T>
Vector3<T> Vector3<T>::rotate(const Vector3<T>& axis, const T& angle) const
{
	Vector3<T> result;

	T sinHalfAngle = std::sin(ToRadians(angle / 2));
	T cosHalfAngle = std::cos(ToRadians(angle / 2));

	T rx = axis.x * sinHalfAngle;
	T ry = axis.y * sinHalfAngle;
	T rz = axis.z * sinHalfAngle;
	T rw = cosHalfAngle;

	Quaternion<T> rotation(rx, ry, rz, rw);
	Quaternion<T> conjugate = rotation.Conjugate();

	Quaternion<T> w = rotation * (*this) * conjugate;

	result.x = w.x;
	result.y = w.y;
	result.z = w.z;

	return result;
}

template<typename T>
inline Vector4<T> Vector4<T>::abs() const
{
	return Vector4<T>(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

template<typename T>
Vector4<T> Vector4<T>::lerp(const Vector4<T>& b, const T& t) const
{
	float x_ = x + (b.x - x) * t;
	float y_ = y + (b.y - y) * t;
	float z_ = z + (b.z - z) * t;
	float w_ = w + (b.w - w) * t;

	return Vector4<T>(x_, y_, z_, w_);
}

template<typename T>
Vector4<T> Vector4<T>::lerp(const Vector4<T>& b, const Vector4<T>& t) const
{
	float x_ = x + (b.x - x) * t.x;
	float y_ = y + (b.y - y) * t.y;
	float z_ = z + (b.z - z) * t.z;
	float w_ = w + (b.w - w) * t.w;

	return Vector4<T>(x_, y_, z_, w_);
}
template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Quaternion<T>& r) const
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	return Quaternion(x_, y_, z_, w_);
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Vector4<T>& r) const
{
	float x_ = x * r.w + w * r.x + y * r.z - z * r.y;
	float y_ = y * r.w + w * r.y + z * r.x - x * r.z;
	float z_ = z * r.w + w * r.z + x * r.y - y * r.x;
	float w_ = w * r.w - x * r.x - y * r.y - z * r.z;

	return Quaternion(x_, y_, z_, w_);
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Vector3<T>& r) const
{
	float x_ = w * r.x + y * r.z - z * r.y;
	float y_ = w * r.y + z * r.x - x * r.z;
	float z_ = w * r.z + x * r.y - y * r.x;
	float w_ = -x * r.x - y * r.y - z * r.z;

	return Quaternion(x_, y_, z_, w_);
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*=(const Quaternion<T>& r) const
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

template<typename T>
Quaternion<T> Quaternion<T>::operator*=(const Vector4<T>& r) const
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

template<typename T>
Quaternion<T> Quaternion<T>::operator*=(const Vector3<T>& r) const
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

template<typename T>
inline Quaternion<T> Quaternion<T>::abs() const
{
	return Quaternion<T>(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

template<typename T>
Quaternion<T> Quaternion<T>::lerp(const Quaternion<T>& b, const T& t) const
{
	float x_ = x + (b.x - x) * t;
	float y_ = y + (b.y - y) * t;
	float z_ = z + (b.z - z) * t;
	float w_ = w + (b.w - w) * t;

	return Quaternion<T>(x_, y_, z_, w_);
}

template<typename T>
Quaternion<T> Quaternion<T>::lerp(const Quaternion<T>& b, const Quaternion<T>& t) const
{
	float x_ = x + (b.x - x) * t.x;
	float y_ = y + (b.y - y) * t.y;
	float z_ = z + (b.z - z) * t.z;
	float w_ = w + (b.w - w) * t.w;

	return Quaternion<T>(x_, y_, z_, w_);
}

#endif