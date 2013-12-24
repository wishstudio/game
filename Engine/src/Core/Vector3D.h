#pragma once

class Vector3D
{
public:
	union {
		struct {
			f32 x, y, z;
		};
		f32 m[3];
	};

	Vector3D() : x(0), y(0), z(0) {}
	Vector3D(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}

	Vector3D &operator= (const Vector3D &vec) { x = vec.x, y = vec.y, z = vec.z; return *this; }

	Vector3D operator+ (const Vector3D &vec) const
	{
		return Vector3D(x + vec.x, y + vec.y, z + vec.z);
	}

	Vector3D &operator+= (const Vector3D &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	Vector3D operator- () const
	{
		return Vector3D(-x, -y, -z);
	}

	Vector3D operator- (const Vector3D &vec) const
	{
		return Vector3D(x - vec.x, y - vec.y, z - vec.z);
	}

	Vector3D &operator-= (const Vector3D &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	Vector3D operator* (f32 scalar) const
	{
		return Vector3D(x * scalar, y * scalar, z * scalar);
	}

	friend Vector3D operator* (f32 scalar, const Vector3D &vec)
	{
		return Vector3D(scalar * vec.x, scalar * vec.y, scalar * vec.z);
	}

	Vector3D &operator*= (f32 scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vector3D operator/ (f32 scalar) const
	{
		return Vector3D(x / scalar, y / scalar, z / scalar);
	}

	Vector3D &operator/= (f32 scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	f32 getLength() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vector3D getNormalized() const
	{
		f32 len = getLength();
		return Vector3D(x / len, y / len, z / len);
	}

	Vector3D getInverted() const
	{
		return Vector3D(-x, -y, -z);
	}

	Vector3D crossProduct(const Vector3D &vec) const
	{
		return Vector3D(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
	}

	f32 dotProduct(const Vector3D &vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}
};
