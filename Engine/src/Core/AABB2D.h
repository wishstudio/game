#pragma once

#include "Size2D.h"
#include "Vector2D.h"

class AABB2D
{
public:
	Vector2D minPoint, maxPoint;

	AABB2D() : minPoint({ 0, 0 }), maxPoint({ -1, -1 }) {}
	AABB2D(const AABB2D &) = default;

	AABB2D &operator= (const AABB2D &) = default;

	bool isValid() const { return maxPoint.x >= minPoint.x && maxPoint.y >= minPoint.y; }
	bool isEmpty() const { return maxPoint.x <= minPoint.x || maxPoint.y <= minPoint.y; }
	f32 getWidth() const { return maxPoint.x - minPoint.x; }
	f32 getHeight() const { return maxPoint.y - minPoint.y; }
	Size2D getSize() const { return Size2D(getWidth(), getHeight()); }
};
