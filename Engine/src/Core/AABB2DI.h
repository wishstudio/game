#pragma once

#include "Size2DI.h"
#include "Vector2DI.h"

class AABB2DI
{
public:
	Vector2DI minPoint, maxPoint;

	AABB2DI(): minPoint({0, 0}), maxPoint({-1, -1}) {}
	AABB2DI(const AABB2DI &) = default;

	AABB2DI &operator= (const AABB2DI &) = default;
	
	bool isValid() const { return maxPoint.x >= minPoint.x && maxPoint.y >= minPoint.y; }
	bool isEmpty() const { return maxPoint.x <= minPoint.x || maxPoint.y <= minPoint.y; }
	s32 getWidth() const { return maxPoint.x - minPoint.x; }
	s32 getHeight() const { return maxPoint.y - minPoint.y; }
	Size2DI getSize() const { return Size2DI(getWidth(), getHeight()); }
};
