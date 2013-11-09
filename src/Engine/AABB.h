#pragma once

#include "Matrix4.h"
#include "Vector3.h"

class AABB
{
public:
	Vector3 minPoint, maxPoint;

	AABB() = delete;
	AABB(const AABB &) = default;
	AABB(const Vector3 &point): minPoint(point), maxPoint(point) {}
	AABB(const Vector3 &_minPoint, const Vector3 &_maxPoint): minPoint(_minPoint), maxPoint(_maxPoint) {}
	AABB(f32 xMin, f32 yMin, f32 zMin, f32 xMax, f32 yMax, f32 zMax): minPoint(xMin, yMin, zMin), maxPoint(xMax, yMax, zMax) {}

	AABB &operator= (const AABB &) = default;

	void merge(const Vector3 &point)
	{
		if (point.x > maxPoint.x)
			maxPoint.x = point.x;
		else if (point.x < minPoint.x)
			minPoint.x = point.x;

		if (point.y > maxPoint.y)
			maxPoint.y = point.y;
		else if (point.y < minPoint.y)
			minPoint.y = point.y;

		if (point.z > maxPoint.z)
			maxPoint.z = point.z;
		else if (point.z < minPoint.z)
			minPoint.z = point.z;
	}

	void merge(const AABB &aabb)
	{
		merge(aabb.minPoint);
		merge(aabb.maxPoint);
	}

	AABB transform(const Matrix4 &matrix) const
	{
		Vector3 rMin(matrix._41, matrix._42, matrix._43);
		Vector3 rMax(matrix._41, matrix._42, matrix._43);
		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < 3; i++)
			{
				const f32 a = matrix.m[i][j] * minPoint.m[j];
				const f32 b = matrix.m[i][j] * maxPoint.m[j];
				if (a < b)
				{
					rMin.m[j] += a;
					rMax.m[j] += b;
				}
				else
				{
					rMin.m[j] += b;
					rMax.m[j] += a;
				}
			}
		}
		return AABB(rMin, rMax);
	}

	AABB translate(f32 offsetX, f32 offsetY, f32 offsetZ) const
	{
		return AABB(minPoint + Vector3(offsetX, offsetY, offsetZ), maxPoint + Vector3(offsetX, offsetY, offsetZ));
	}
};
