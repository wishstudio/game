#include <Core.h>

Matrix4 Matrix4::translation(const Vector3D &offset)
{
	return translation(offset.x, offset.y, offset.z);
}

Matrix4 Matrix4::inverseTranslation(const Vector3D &offset)
{
	return inverseTranslation(offset.x, offset.y, offset.z);
}

Matrix4 Matrix4::lookAtLH(const Vector3D &eye, const Vector3D &at, const Vector3D &up)
{
	Vector3D zaxis = (at - eye).getNormalized();
	Vector3D xaxis = up.crossProduct(zaxis).getNormalized();
	Vector3D yaxis = zaxis.crossProduct(xaxis);

	return Matrix4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-xaxis.dotProduct(eye), -yaxis.dotProduct(eye), -zaxis.dotProduct(eye), 1);
}

Matrix4 Matrix4::lookAtRH(const Vector3D &eye, const Vector3D &at, const Vector3D &up)
{
	Vector3D zaxis = (eye - at).getNormalized();
	Vector3D xaxis = up.crossProduct(zaxis).getNormalized();
	Vector3D yaxis = zaxis.crossProduct(xaxis);

	return Matrix4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-xaxis.dotProduct(eye), -yaxis.dotProduct(eye), -zaxis.dotProduct(eye), 1);
}
