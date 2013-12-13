#pragma once

class Size2DI
{
public:
	s32 width, height;

	Size2DI(): width(-1), height(-1) {}
	Size2DI(s32 _width, s32 _height): width(_width), height(_height) {}
	Size2DI(const Size2DI &) = default;

	Size2DI &operator= (const Size2DI &) = default;

	bool isValid() const { return width >= 0 && height >= 0; }
	bool isEmpty() const { return width <= 0 || height <= 0; }
};
