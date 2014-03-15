#pragma once

class Colorf;
class Color
{
public:
	u8 r, g, b, a;

	Color() : r(0), g(0), b(0), a(0) {}
	Color(u8 _r, u8 _g, u8 _b, u8 _a) : r(_r), g(_g), b(_b), a(_a) {}

	Color(const Colorf &c);
	Color &operator= (const Colorf &c);
};

class Colorf
{
public:
	float r, g, b, a;

	Colorf() : r(0), g(0), b(0), a(0) {}
	Colorf(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}

	Colorf(const Colorf &c) = default;
	Colorf &operator= (const Colorf &c) = default;
	Colorf(const Color &c);
	Colorf &operator= (const Color &c);
	
	Colorf operator+ (const Colorf &c)
	{
		return Colorf(r + c.r, g + c.g, b + c.b, a + c.a);
	}

	Colorf &operator+= (const Colorf &c)
	{
		r += c.r;
		g += c.g;
		b += c.b;
		a += c.a;
		return *this;
	}

	Colorf operator- (const Colorf &c)
	{
		return Colorf(r - c.r, g - c.g, b - c.b, a - c.a);
	}

	Colorf &operator-= (const Colorf &c)
	{
		r -= c.r;
		g -= c.g;
		b -= c.b;
		a -= c.a;
		return *this;
	}

	Colorf operator* (float scalar)
	{
		return Colorf(r * scalar, g * scalar, b * scalar, a * scalar);
	}

	Colorf &operator*= (float scalar)
	{
		r *= scalar;
		g *= scalar;
		b *= scalar;
		a *= scalar;
		return *this;
	}

	Colorf operator/ (float scalar)
	{
		return Colorf(r / scalar, g / scalar, b / scalar, a / scalar);
	}

	Colorf &operator/= (float scalar)
	{
		r /= scalar;
		g /= scalar;
		b /= scalar;
		a /= scalar;
		return *this;
	}
};

inline Color::Color(const Colorf &c)
{
	r = (u8)(c.r * 0xFF);
	g = (u8)(c.g * 0xFF);
	b = (u8)(c.b * 0xFF);
	a = (u8)(c.a * 0xFF);
}

inline Color &Color::operator= (const Colorf &c)
{
	r = (u8)(c.r * 0xFF);
	g = (u8)(c.g * 0xFF);
	b = (u8)(c.b * 0xFF);
	a = (u8)(c.a * 0xFF);
	return *this;
}

inline Colorf::Colorf(const Color &c)
{
	r = c.r / (float)0xFF;
	g = c.g / (float)0xFF;
	b = c.b / (float)0xFF;
	a = c.a / (float)0xFF;
}

inline Colorf &Colorf::operator= (const Color &c)
{
	r = c.r / (float)0xFF;
	g = c.g / (float)0xFF;
	b = c.b / (float)0xFF;
	a = c.a / (float)0xFF;
	return *this;
}
