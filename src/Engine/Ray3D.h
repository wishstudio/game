#pragma once

#include "Vector3D.h"

class Ray3D
{
public:
	Vector3D start, direction;

	Ray3D() = default;
	Ray3D(const Ray3D &) = default;
	Ray3D(const Vector3D &_start, const Vector3D &_direction): start(_start), direction(_direction) {}

	Ray3D &operator= (const Ray3D &) = default;
};
