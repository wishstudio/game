#pragma once

#include "Matrix4.h"

class Vector3
{
public:
	union {
		struct {
			f32 x, y, z;
		};
		f32 m[3];
	};

	Vector3() : x(0), y(0), z(0) {}
	Vector3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}

	Vector3 &operator= (const Vector3 &vec) { x = vec.x, y = vec.y, z = vec.z; return *this; }

	Vector3 operator+ (const Vector3 &vec) const
	{
		return Vector3(x + vec.x, y + vec.y, z + vec.z);
	}

	Vector3 &operator+= (const Vector3 &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	Vector3 operator- () const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 operator- (const Vector3 &vec) const
	{
		return Vector3(x - vec.x, y - vec.y, z - vec.z);
	}

	Vector3 &operator-= (const Vector3 &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	Vector3 operator* (f32 scalar) const
	{
		return Vector3(x * scalar, y * scalar, z * scalar);
	}

	friend Vector3 operator* (f32 scalar, const Vector3 &vec)
	{
		return Vector3(scalar * vec.x, scalar * vec.y, scalar * vec.z);
	}

	Vector3 &operator*= (f32 scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vector3 operator/ (f32 scalar) const
	{
		return Vector3(x / scalar, y / scalar, z / scalar);
	}

	Vector3 &operator/= (f32 scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	Vector3 Vector3::operator* (const Matrix4 &mat) const
	{
		f32 _x = x * mat._11 + y * mat._21 + z * mat._31 + mat._41;
		f32 _y = x * mat._12 + y * mat._22 + z * mat._32 + mat._42;
		f32 _z = x * mat._13 + y * mat._23 + z * mat._33 + mat._43;
		return Vector3(_x, _y, _z);
	}

	f32 getLength() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vector3 getNormalized() const
	{
		f32 len = getLength();
		return Vector3(x / len, y / len, z / len);
	}

	Vector3 getInverted() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 crossProduct(const Vector3 &vec) const
	{
		return Vector3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
	}

	f32 dotProduct(const Vector3 &vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}
};
