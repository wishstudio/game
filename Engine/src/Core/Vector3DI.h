#pragma once

class Vector3DI
{
public:
	union {
		struct {
			s32 x, y, z;
		};
		s32 m[3];
	};

	Vector3DI():  x(0), y(0), z(0) {}
	Vector3DI(s32 _x, s32 _y, s32 _z) : x(_x), y(_y), z(_z) {}

	Vector3DI &operator= (const Vector3DI &vec) = default;

	Vector3DI operator+ (const Vector3DI &vec) const
	{
		return Vector3DI(x + vec.x, y + vec.y, z + vec.z);
	}

	Vector3DI &operator+= (const Vector3DI &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	Vector3DI operator- () const
	{
		return Vector3DI(-x, -y, -z);
	}

	Vector3DI operator- (const Vector3DI &vec) const
	{
		return Vector3DI(x - vec.x, y - vec.y, z - vec.z);
	}

	Vector3DI &operator-= (const Vector3DI &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	Vector3DI operator* (s32 scalar) const
	{
		return Vector3DI(x * scalar, y * scalar, z * scalar);
	}

	friend Vector3DI operator* (s32 scalar, const Vector3DI &vec)
	{
		return Vector3DI(scalar * vec.x, scalar * vec.y, scalar * vec.z);
	}

	Vector3DI &operator*= (s32 scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vector3DI operator/ (s32 scalar) const
	{
		return Vector3DI(x / scalar, y / scalar, z / scalar);
	}

	Vector3DI &operator/= (s32 scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
};
