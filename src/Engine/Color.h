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
	f32 r, g, b, a;

	Colorf() : r(0), g(0), b(0), a(0) {}
	Colorf(f32 _r, f32 _g, f32 _b, f32 _a) : r(_r), g(_g), b(_b), a(_a) {}

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

	Colorf operator* (f32 scalar)
	{
		return Colorf(r * scalar, g * scalar, b * scalar, a * scalar);
	}

	Colorf &operator*= (f32 scalar)
	{
		r *= scalar;
		g *= scalar;
		b *= scalar;
		a *= scalar;
		return *this;
	}

	Colorf operator/ (f32 scalar)
	{
		return Colorf(r / scalar, g / scalar, b / scalar, a / scalar);
	}

	Colorf &operator/= (f32 scalar)
	{
		r /= scalar;
		g /= scalar;
		b /= scalar;
		a /= scalar;
		return *this;
	}
};

Color::Color(const Colorf &c)
{
	r = c.r * 0xFF;
	g = c.g * 0xFF;
	b = c.b * 0xFF;
	a = c.a * 0xFF;
}

Color &Color::operator= (const Colorf &c)
{
	r = c.r * 0xFF;
	g = c.g * 0xFF;
	b = c.b * 0xFF;
	a = c.a * 0xFF;
	return *this;
}

Colorf::Colorf(const Color &c)
{
	r = c.r / (f32)0xFF;
	g = c.g / (f32)0xFF;
	b = c.b / (f32)0xFF;
	a = c.a / (f32)0xFF;
}

Colorf &Colorf::operator= (const Color &c)
{
	r = c.r / (f32)0xFF;
	g = c.g / (f32)0xFF;
	b = c.b / (f32)0xFF;
	a = c.a / (f32)0xFF;
	return *this;
}
