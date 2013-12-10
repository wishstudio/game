#pragma once

class Vector2D
{
public:
	union
	{
		struct
		{
			f32 x, y;
		};
		f32 m[2];
	};

	Vector2D(): x(0), y(0) {}
	Vector2D(const Vector2D &) = default;
	Vector2D(f32 _x, f32 _y): x(_x), y(_y) {}

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

	Vector2D operator* (f32 scale) const
	{
		return Vector2D(x * scale, y * scale);
	}

	Vector2D &operator*= (f32 scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	Vector2D operator/ (f32 scale) const
	{
		return Vector2D(x / scale, y / scale);
	}

	Vector2D &operator/= (f32 scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}
};
