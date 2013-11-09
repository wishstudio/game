#include <stdafx.h>

#include "Matrix4.h"
#include "Vector3.h"

Matrix4 Matrix4::translation(const Vector3 &offset)
{
	return translation(offset.x, offset.y, offset.z);
}

Matrix4 Matrix4::inverseTranslation(const Vector3 &offset)
{
	return inverseTranslation(offset.x, offset.y, offset.z);
}

Matrix4 Matrix4::lookAtLH(const Vector3 &eye, const Vector3 &at, const Vector3 &up)
{
	Vector3 zaxis = (at - eye).getNormalized();
	Vector3 xaxis = up.crossProduct(zaxis).getNormalized();
	Vector3 yaxis = zaxis.crossProduct(xaxis);

	return Matrix4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-xaxis.dotProduct(eye), -yaxis.dotProduct(eye), -zaxis.dotProduct(eye), 1);
}

static Matrix4 lookAtRH(const Vector3 &eye, const Vector3 &at, const Vector3 &up)
{
	Vector3 zaxis = (eye - at).getNormalized();
	Vector3 xaxis = up.crossProduct(zaxis).getNormalized();
	Vector3 yaxis = zaxis.crossProduct(xaxis);

	return Matrix4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		xaxis.dotProduct(eye), yaxis.dotProduct(eye), zaxis.dotProduct(eye), 1);
}

void Matrix4::transformBox(aabbox3df &box)
{
	Vector3 vMin(box.MinEdge);
	Vector3 vMax(box.MaxEdge);
	Vector3 rMin(_41, _42, _43);
	Vector3 rMax(_41, _42, _43);
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			const f32 a = m[i][j] * vMin.m[j];
			const f32 b = m[i][j] * vMax.m[j];
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
	box.MinEdge = rMin;
	box.MaxEdge = rMax;
}
