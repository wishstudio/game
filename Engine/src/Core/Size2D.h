#pragma once

class Size2D
{
public:
	float width, height;

	Size2D() : width(-1), height(-1) {}
	Size2D(float _width, float _height) : width(_width), height(_height) {}
	Size2D(const Size2D &) = default;

	Size2D &operator= (const Size2D &) = default;

	bool isValid() const { return width >= 0 && height >= 0; }
	bool isEmpty() const { return width <= 0 || height <= 0; }
};
