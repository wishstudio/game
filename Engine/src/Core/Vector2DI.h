#pragma once

class Vector2DI
{
public:
	union
	{
		struct
		{
			s32 x, y;
		};
		s32 m[2];
	};

	Vector2DI() : x(0), y(0) {}
	Vector2DI(const Vector2DI &) = default;
	Vector2DI(s32 _x, s32 _y) : x(_x), y(_y) {}

	Vector2DI &operator= (const Vector2DI &) = default;

	Vector2DI operator+ (const Vector2DI &vec) const
	{
		return Vector2DI(x + vec.x, y + vec.y);
	}

	Vector2DI &operator+= (const Vector2DI &vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}

	Vector2DI operator- (const Vector2DI &vec) const
	{
		return Vector2DI(x - vec.x, y - vec.y);
	}

	Vector2DI &operator-= (const Vector2DI &vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	Vector2DI operator* (s32 scale) const
	{
		return Vector2DI(x * scale, y * scale);
	}

	Vector2DI &operator*= (s32 scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	Vector2DI operator/ (s32 scale) const
	{
		return Vector2DI(x / scale, y / scale);
	}

	Vector2DI &operator/= (s32 scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}
};
