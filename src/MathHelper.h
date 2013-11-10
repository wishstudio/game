#pragma once

#include <limits>

template<class T>
inline const T &min(const T &a, const T &b)
{
	return a < b? a: b;
}

template<class T>
inline const T &max(const T &a, const T &b)
{
	return a > b? a: b;
}

template<class T>
inline const T &abs(const T &a)
{
	return a >= 0? a: -a;
}

template<class T>
inline const T &bound(const T &a, const T &b, const T &c)
{
	return min(max(a, b), c);
}

template<class T>
inline int sgn(const T &a)
{
	if (a > 0)
		return 1;
	else if (a < 0)
		return -1;
	else
		return 0;
}

template<class T>
inline T mod(const T &a, const T &b)
{
	T ret = a % b;
	if (ret < 0)
		ret += b;
	return ret;
}

template<typename T>
inline T divide(const T &a, const T &b)
{
	if (a < 0)
		return (a - (b - 1)) / b;
	else
		return a / b;
}

inline bool rayIntersectsWithBox(const Ray3D &ray, const AABB &box)
{
	/* MinX <= Xt + x0 <= MaxX
	   MinY <= Yt + y0 <= MaxY
	   MinZ <= Zt + z0 <= MinZ */
	f32 x0 = ray.start.x, X = ray.direction.x;
	f32 y0 = ray.start.y, Y = ray.direction.y;
	f32 z0 = ray.start.z, Z = ray.direction.z;

	/* Note the following formulas (derived from above)
	   Xt >= MinX - x0
	   Xt <= MaxX - x0
	   Yt >= MinY - y0
	   Yt <= MaxY - y0
	   Zt >= MinZ - z0
	   Zt <= MaxZ - z0
	   It is obvious the range of t can be determined.
	 */
	f32 tMin = 0, tMax = std::numeric_limits<f32>::infinity();
	if (isZero(X)) /* 0 >= MinX - x0 */ /* 0 <= MaxX - x0 */
	{
		if (box.minPoint.x - x0 > EPSILON || box.maxPoint.x < -EPSILON)
			return false;
	}
	else if (X > 0)
	{
		tMin = max(tMin, (box.minPoint.x - x0) / X);
		tMax = min(tMax, (box.maxPoint.x - x0) / X);
	}
	else
	{
		tMin = max(tMin, (box.maxPoint.x - x0) / X);
		tMax = min(tMax, (box.minPoint.x - x0) / X);
	}

	if (isZero(Y))
	{
		if (box.minPoint.y - y0 > EPSILON || box.maxPoint.y < -EPSILON)
			return false;
	}
	else if (Y > 0)
	{
		tMin = max(tMin, (box.minPoint.y - y0) / Y);
		tMax = min(tMax, (box.maxPoint.y - y0) / Y);
	}
	else
	{
		tMin = max(tMin, (box.maxPoint.y - y0) / Y);
		tMax = min(tMax, (box.minPoint.y - y0) / Y);
	}

	if (isZero(Z))
	{
		if (box.minPoint.z - z0 > EPSILON || box.maxPoint.z < -EPSILON)
			return false;
	}
	else if (Z > 0)
	{
		tMin = max(tMin, (box.minPoint.z - z0) / Z);
		tMax = min(tMax, (box.maxPoint.z - z0) / Z);
	}
	else
	{
		tMin = max(tMin, (box.maxPoint.z - z0) / Z);
		tMax = min(tMax, (box.minPoint.z - z0) / Z);
	}
	return tMin <= tMax;
}

inline bool rayIntersectsWithSphere(const Ray3D &ray, const Vector3 &center, f32 radius, f32 &distance)
{
	/*         |
	 *         |       % % %
     *         |    %ray      %
	 *         |   % /'.       %
	 *         |    /   'Q
     *         | v %   ,'      %
     *         |  / %,' c     %
     *         | /a,'  % % %
	 *         |/,'  
	 *  -------+--------------------------
	 *         |
	 *         |
	 */
	/* Sphere center in point coordinate system */
	Vector3 Q = center - ray.start;
	/* Distance from sphere center to origin */
	f32 c = Q.getLength();
	/* v = Q * vn
	     = |Q| * 1 * cos(alpha)
	     = c * cos(alpha)
	 */
	f32 v = Q.dotProduct(ray.direction.getNormalized());
	if (v < 0.f)
		return false;
	/* d = R^2 - (c^2 - v^2)
	 *   = R^2 - (c^2 - (c * cos(alpha))^2)
	 *   = R^2 - c^2 * (1 - cos(alpha)^2)
	 *   = R^2 - c^2 * sin(alpha)^2
	 *   = R^2 - (c * sin(alpha))^2
	 */
	f32 d = radius * radius - (c * c - v * v);
	if (d < 0.f)
		return false;

	distance = v - sqrt(d);
	return true;
}

inline bool rayIntersectsPlane(const Ray3D &ray, const Triangle3D &plane, Vector3 &intersectionPoint)
{
	/* Plane: n * X = d
	 * Line : X = P + tV
	 * =>   n * (P + tV) = d
	 * => n * P + tn * V = d
	 * =>         tn * V = d - n * P
	 * =>              t = (d - n * P) / (n * V)
	 */
	Vector3 n = plane.getNormal().getNormalized();
	f32 d = plane.pointA.dotProduct(n);
	f32 t = (d - n.dotProduct(ray.start)) / n.dotProduct(ray.direction);
	/* If n * V == 0 we will get Inf or NaN, they are correctly handled below */
	if (t > -EPSILON && t < std::numeric_limits<f32>::infinity())
	{
		intersectionPoint = ray.start + t * ray.direction;
		return true;
	}
	return false;
}

enum Direction
{
	DIRECTION_X = 0,
	DIRECTION_Y = 1,
	DIRECTION_Z = 2,
	DIRECTION_MX = 3,
	DIRECTION_MY = 4,
	DIRECTION_MZ = 5,
	DIRECTION_COUNT = 6
};

static const int dirX[DIRECTION_COUNT] = { 1, 0, 0, -1,  0,  0 };
static const int dirY[DIRECTION_COUNT] = { 0, 1, 0,  0, -1,  0 };
static const int dirZ[DIRECTION_COUNT] = { 0, 0, 1,  0,  0, -1 };

inline Direction oppositeDirection(Direction direction)
{
	if ((int) direction < 3)
		return (Direction) ((int) direction + 3);
	else
		return (Direction) ((int) direction - 3);
}

static const int CHUNK_SIZE = 16;
static_assert((CHUNK_SIZE & (CHUNK_SIZE - 1)) == 0, "CHUNK_SIZE must be power of 2.");

inline bool isInChunk(int in_x, int in_y, int in_z)
{
	return (in_x & (CHUNK_SIZE - 1)) == in_x &&
		(in_y & (CHUNK_SIZE - 1)) == in_y &&
		(in_z & (CHUNK_SIZE - 1)) == in_z;
}