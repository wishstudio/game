#pragma once

#include "Vector3.h"

class Ray3D
{
public:
	Vector3 start, direction;

	Ray3D() = default;
	Ray3D(const Ray3D &) = default;
	Ray3D(const Vector3 &_start, const Vector3 &_direction): start(_start), direction(_direction) {}

	Ray3D &operator= (const Ray3D &) = default;
};
