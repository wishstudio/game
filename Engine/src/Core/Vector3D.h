#pragma once

class Vector3D
{
public:
	union {
		struct {
			float x, y, z;
		};
		float m[3];
	};

	Vector3D() : x(0), y(0), z(0) {}
	Vector3D(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

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

	Vector3D operator* (float scalar) const
	{
		return Vector3D(x * scalar, y * scalar, z * scalar);
	}

	friend Vector3D operator* (float scalar, const Vector3D &vec)
	{
		return Vector3D(scalar * vec.x, scalar * vec.y, scalar * vec.z);
	}

	Vector3D &operator*= (float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vector3D operator/ (float scalar) const
	{
		return Vector3D(x / scalar, y / scalar, z / scalar);
	}

	Vector3D &operator/= (float scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	float getLength() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vector3D getNormalized() const
	{
		float len = getLength();
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

	float dotProduct(const Vector3D &vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}
};
