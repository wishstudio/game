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

template<typename DestT, typename SrcT>
inline vector3d<DestT> vector3d_cast(const vector3d<SrcT> &source)
{
	return vector3d<DestT>(source.X, source.Y, source.Z);
}

inline void translateBox(aabbox3df &box, int offsetX, int offsetY, int offsetZ)
{
	box.MinEdge.X += offsetX;
	box.MinEdge.Y += offsetY;
	box.MinEdge.Z += offsetZ;

	box.MaxEdge.X += offsetX;
	box.MaxEdge.Y += offsetY;
	box.MaxEdge.Z += offsetZ;
}

inline bool rayIntersectsWithBox(const line3df &ray, const aabbox3df &box)
{
	/* MinX <= Xt + x0 <= MaxX
	   MinY <= Yt + y0 <= MaxY
	   MinZ <= Zt + z0 <= MinZ */
	f32 x0 = ray.start.X, X = ray.end.X - ray.start.X;
	f32 y0 = ray.start.Y, Y = ray.end.Y - ray.start.Y;
	f32 z0 = ray.start.Z, Z = ray.end.Z - ray.start.Z;

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
	if (iszero(X)) /* 0 >= MinX - x0 */ /* 0 <= MaxX - x0 */
	{
		if (box.MinEdge.X - x0 > ROUNDING_ERROR_f32 || box.MaxEdge.X < -ROUNDING_ERROR_f32)
			return false;
	}
	else if (X > 0)
	{
		tMin = max(tMin, (box.MinEdge.X - x0) / X);
		tMax = min(tMax, (box.MaxEdge.X - x0) / X);
	}
	else
	{
		tMin = max(tMin, (box.MaxEdge.X - x0) / X);
		tMax = min(tMax, (box.MinEdge.X - x0) / X);
	}

	if (iszero(Y))
	{
		if (box.MinEdge.Y - y0 > ROUNDING_ERROR_f32 || box.MaxEdge.Y < -ROUNDING_ERROR_f32)
			return false;
	}
	else if (Y > 0)
	{
		tMin = max(tMin, (box.MinEdge.Y - y0) / Y);
		tMax = min(tMax, (box.MaxEdge.Y - y0) / Y);
	}
	else
	{
		tMin = max(tMin, (box.MaxEdge.Y - y0) / Y);
		tMax = min(tMax, (box.MinEdge.Y - y0) / Y);
	}

	if (iszero(Z))
	{
		if (box.MinEdge.Z - z0 > ROUNDING_ERROR_f32 || box.MaxEdge.Z < -ROUNDING_ERROR_f32)
			return false;
	}
	else if (Z > 0)
	{
		tMin = max(tMin, (box.MinEdge.Z - z0) / Z);
		tMax = min(tMax, (box.MaxEdge.Z - z0) / Z);
	}
	else
	{
		tMin = max(tMin, (box.MaxEdge.Z - z0) / Z);
		tMax = min(tMax, (box.MinEdge.Z - z0) / Z);
	}
	return tMin <= tMax;
}

inline bool rayIntersectsWithSphere(const Vector3 &point, const Vector3 &vec, const Vector3 &center, f32 radius, f32 &distance)
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
	Vector3 Q = center - point;
	/* Distance from sphere center to origin */
	f32 c = Q.getLength();
	/* v = Q * vn
	     = |Q| * 1 * cos(alpha)
	     = c * cos(alpha)
	 */
	Vector3 vn(vec);
	f32 v = Q.dotProduct(vn.getNormalized());
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