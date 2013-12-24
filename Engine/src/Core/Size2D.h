#pragma once

class Size2D
{
public:
	f32 width, height;

	Size2D() : width(-1), height(-1) {}
	Size2D(f32 _width, f32 _height) : width(_width), height(_height) {}
	Size2D(const Size2D &) = default;

	Size2D &operator= (const Size2D &) = default;

	bool isValid() const { return width >= 0 && height >= 0; }
	bool isEmpty() const { return width <= 0 || height <= 0; }
};
