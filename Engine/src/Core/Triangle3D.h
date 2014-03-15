#pragma once

#include "Vector3D.h"

class Triangle3D
{
public:
	Vector3D pointA, pointB, pointC;

	Triangle3D() = default;
	Triangle3D(const Triangle3D &) = default;
	/* Order must be clockwise in left-handed system, or counter-clockwise in right-handed system. */
	Triangle3D(const Vector3D &_pointA, const Vector3D &_pointB, const Vector3D &_pointC):
		pointA(_pointA), pointB(_pointB), pointC(_pointC)
	{}

	Triangle3D &operator= (const Triangle3D &) = default;

	/* Be warned this may not be normalized */
	Vector3D getNormal() const
	{
		/* This should have higher precision than simple (B-A)x(C-A) */
		float nx = pointA.y * (pointB.z - pointC.z) + pointB.y * (pointC.z - pointA.z) + pointC.y * (pointA.z - pointB.z);
		float ny = pointA.z * (pointB.x - pointC.x) + pointB.z * (pointC.x - pointA.x) + pointC.z * (pointA.x - pointB.x);
		float nz = pointA.x * (pointB.y - pointC.y) + pointB.x * (pointC.y - pointA.y) + pointC.x * (pointA.y - pointB.y);
		return Vector3D(nx, ny, nz);
	}

	bool isFrontFacing(const Vector3D &lookDirection) const
	{
		float d = getNormal().dotProduct(lookDirection);
		return d < EPSILON;
	}

	/* Point must lie on the plane of the triangle */
	bool isPointInside(const Vector3D &point) const
	{
		/* Translate triangle so that point lies at origin */
		Vector3D a = pointA - point, b = pointB - point, c = pointC - point;
		/* Compute normal for triangle PAB and triangle PBC */
		Vector3D u = a.crossProduct(b);
		Vector3D v = b.crossProduct(c);
		/* Make sure they are both pointing in the same direction */
		if (u.dotProduct(v) < -EPSILON)
			return false;
		/* Compute normal for triangle PCA */
		Vector3D w = c.crossProduct(a);
		/* Make sure it points in the same direction as the first two */
		if (u.dotProduct(w) < -EPSILON)
			return false;
		/* P must be in (or on) the triangle */
		return true;
	}

	/* Point must lie on the plane of the triangle and be outside the triangle */
	Vector3D closestPointOnTriangle(const Vector3D &point) const
	{
		Vector3D ab = closestPointOnLine(point, pointA, pointB);
		Vector3D bc = closestPointOnLine(point, pointB, pointC);
		Vector3D ca = closestPointOnLine(point, pointC, pointA);

		float dab = (ab - point).getLength();
		float dbc = (bc - point).getLength();
		float dca = (ca - point).getLength();
		
		if (dab < dbc)
			return dab < dca? ab: ca;
		return dbc < dca? bc: ca;
	}

	/* TODO */
	Vector3D closestPointOnLine(const Vector3D &P, const Vector3D &A, const Vector3D &B) const
	{
		Vector3D V = (B - A).getNormalized();
		float t = (P - A).dotProduct(V);
		if (t < 0)
			return A;
		if (t > (B - A).getLength())
			return B;
		return A + t * (V);
	}
};
