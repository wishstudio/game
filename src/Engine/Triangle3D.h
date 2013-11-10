#pragma once

#include "Vector3.h"

class Triangle3D
{
public:
	Vector3 pointA, pointB, pointC;

	Triangle3D() = default;
	Triangle3D(const Triangle3D &) = default;
	/* Order must be clockwise in left-handed system, or counter-clockwise in right-handed system. */
	Triangle3D(const Vector3 &_pointA, const Vector3 &_pointB, const Vector3 &_pointC):
		pointA(_pointA), pointB(_pointB), pointC(_pointC)
	{}
	operator triangle3df() const { return triangle3df(pointA, pointB, pointC); }

	Triangle3D &operator= (const Triangle3D &) = default;

	/* Be warned this may not be normalized */
	Vector3 getNormal() const
	{
		/* This should have higher precision than simple (B-A)x(C-A) */
		f32 nx = pointA.y * (pointB.z - pointC.z) + pointB.y * (pointC.z - pointA.z) + pointC.y * (pointA.z - pointB.z);
		f32 ny = pointA.z * (pointB.x - pointC.x) + pointB.z * (pointC.x - pointA.x) + pointC.z * (pointA.x - pointB.x);
		f32 nz = pointA.x * (pointB.y - pointC.y) + pointB.x * (pointC.y - pointA.y) + pointC.x * (pointA.y - pointB.y);
		return Vector3(nx, ny, nz);
	}

	bool isFrontFacing(const Vector3 &lookDirection) const
	{
		f32 d = getNormal().dotProduct(lookDirection);
		return d <= std::numeric_limits<f32>::epsilon(); /* TODO */
	}

	/* Point must lie on the plane of the triangle */
	bool isPointInside(const Vector3 &point) const
	{
		/* Translate triangle so that point lies at origin */
		Vector3 a = pointA - point, b = pointB - point, c = pointC - point;
		/* Compute normal for triangle PAB and triangle PBC */
		Vector3 u = a.crossProduct(b);
		Vector3 v = b.crossProduct(c);
		/* Make sure they are both pointing in the same direction */
		if (u.dotProduct(v) < -std::numeric_limits<f32>::epsilon()) /* TODO */
			return false;
		/* Compute normal for triangle PCA */
		Vector3 w = c.crossProduct(a);
		/* Make sure it points in the same direction as the first two */
		if (u.dotProduct(w) < -std::numeric_limits<f32>::epsilon()) /* TODO */
			return false;
		/* P must be in (or on) the triangle */
		return true;
	}

	/* Point must lie on the plane of the triangle and be outside the triangle */
	Vector3 closestPointOnTriangle(const Vector3 &point) const
	{
		Vector3 ab = closestPointOnLine(point, pointA, pointB);
		Vector3 bc = closestPointOnLine(point, pointB, pointC);
		Vector3 ca = closestPointOnLine(point, pointC, pointA);

		f32 dab = (ab - point).getLength();
		f32 dbc = (bc - point).getLength();
		f32 dca = (ca - point).getLength();
		
		if (dab < dbc)
			return dab < dca? ab: ca;
		return dbc < dca? bc: ca;
	}

	/* TODO */
	Vector3 closestPointOnLine(const Vector3 &P, const Vector3 &A, const Vector3 &B) const
	{
		Vector3 V = (B - A).getNormalized();
		f32 t = (P - A).dotProduct(V);
		if (t < 0)
			return A;
		if (t > (B - A).getLength())
			return B;
		return A + t * (V);
	}
};
