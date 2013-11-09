#pragma once

struct Colorf;
struct Color
{
	union
	{
		u32 argb;
		struct
		{
			u8 b, g, r, a;
		};
	};

	inline Color() : argb(0) {}
	inline Color(u32 _argb) : argb(_argb) {}
	inline Color(u8 _a, u8 _r, u8 _g, u8 _b) : a(_a), r(_r), g(_g), b(_b) {}

	inline Color(const Colorf &c);
	inline Color &operator= (const Colorf &c);
};

struct Colorf
{
	f32 b, g, r, a;

	inline Colorf() : a(0), r(0), g(0), b(0) {}
	inline Colorf(f32 _a, f32 _r, f32 _g, f32 _b) : a(_a), r(_r), g(_g), b(_b) {}
	inline Colorf &operator+= (const Colorf &c) { b += c.b, g += c.g, r += c.r, a += c.a; return *this; }
	inline Colorf &operator-= (const Colorf &c) { b -= c.b, g -= c.g, r -= c.r, a -= c.a; return *this; }
	inline Colorf &operator*= (f32 c) { b *= c, g *= c, r *= c, a *= c; return *this; }
	inline Colorf &operator/= (f32 c) { b /= c, g /= c, r /= c, a /= c; return *this; }
	inline Colorf operator+ (const Colorf &c) { return Colorf(a + c.a, r + c.r, g + c.g, b + c.b); }
	inline Colorf operator- (const Colorf &c) { return Colorf(a - c.a, r - c.r, g - c.g, b - c.b); }
	inline Colorf operator* (f32 c) { return Colorf(a * c, r * c, g * c, b * c); }
	inline Colorf operator/ (f32 c) { return Colorf(a / c, r / c, g / c, b / c); }

	inline Colorf(const Colorf &c) : a(c.a), r(c.r), g(c.g), b(c.b) {}
	inline Colorf &operator= (const Colorf &c) { b = c.b, g = c.g, r = c.r, a = c.a; return *this; }
	inline Colorf(const Color &c);
	inline Colorf &operator= (const Color &c);
};

inline Color::Color(const Colorf &c)
{
	b = c.b * 0xFF;
	g = c.g * 0xFF;
	r = c.r * 0xFF;
	a = c.a * 0xFF;
}

inline Color &Color::operator= (const Colorf &c)
{
	b = c.b * 0xFF;
	g = c.g * 0xFF;
	r = c.r * 0xFF;
	a = c.a * 0xFF;
	return *this;
}

inline Colorf::Colorf(const Color &c)
{
	b = c.b / (f32) 0xFF;
	g = c.g / (f32) 0xFF;
	r = c.r / (f32) 0xFF;
	a = c.a / (f32) 0xFF;
}

inline Colorf &Colorf::operator= (const Color &c)
{
	b = c.b / (f32) 0xFF;
	g = c.g / (f32) 0xFF;
	r = c.r / (f32) 0xFF;
	a = c.a / (f32) 0xFF;
	return *this;
}
