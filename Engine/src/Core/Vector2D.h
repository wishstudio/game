#pragma once

class Vector2D
{
public:
	union
	{
		struct
		{
			float x, y;
		};
		float m[2];
	};

	Vector2D(): x(0), y(0) {}
	Vector2D(const Vector2D &) = default;
	Vector2D(float _x, float _y): x(_x), y(_y) {}

	Vector2D &operator= (const Vector2D &) = default;

	Vector2D operator+ (const Vector2D &vec) const
	{
		return Vector2D(x + vec.x, y + vec.y);
	}

	Vector2D &operator+= (const Vector2D &vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}

	Vector2D operator- (const Vector2D &vec) const
	{
		return Vector2D(x - vec.x, y - vec.y);
	}

	Vector2D &operator-= (const Vector2D &vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	Vector2D operator* (float scale) const
	{
		return Vector2D(x * scale, y * scale);
	}

	Vector2D &operator*= (float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	Vector2D operator/ (float scale) const
	{
		return Vector2D(x / scale, y / scale);
	}

	Vector2D &operator/= (float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}
};
