#pragma once

#include <cmath>

class Vector3D;
class Matrix4
{
public:
	union {
		struct {
			f32 _11, _12, _13, _14;
			f32 _21, _22, _23, _24;
			f32 _31, _32, _33, _34;
			f32 _41, _42, _43, _44;
		};
		f32 m[4][4];
	};

	Matrix4() {}
	Matrix4(
		f32 m11, f32 m12, f32 m13, f32 m14,
		f32 m21, f32 m22, f32 m23, f32 m24,
		f32 m31, f32 m32, f32 m33, f32 m34,
		f32 m41, f32 m42, f32 m43, f32 m44) :
		_11(m11), _12(m12), _13(m13), _14(m14),
		_21(m21), _22(m22), _23(m23), _24(m24),
		_31(m31), _32(m32), _33(m33), _34(m34),
		_41(m41), _42(m42), _43(m43), _44(m44) {}

	Matrix4(const Matrix4 &mat) = default;
	Matrix4 &operator= (const Matrix4 &mat) = default;

	Matrix4 operator* (const Matrix4 &mat) const
	{
		f32 m11 = _11 * mat._11 + _12 * mat._21 + _13 * mat._31 + _14 * mat._41;
		f32 m12 = _11 * mat._12 + _12 * mat._22 + _13 * mat._32 + _14 * mat._42;
		f32 m13 = _11 * mat._13 + _12 * mat._23 + _13 * mat._33 + _14 * mat._43;
		f32 m14 = _11 * mat._14 + _12 * mat._24 + _13 * mat._34 + _14 * mat._44;

		f32 m21 = _21 * mat._11 + _22 * mat._21 + _23 * mat._31 + _24 * mat._41;
		f32 m22 = _21 * mat._12 + _22 * mat._22 + _23 * mat._32 + _24 * mat._42;
		f32 m23 = _21 * mat._13 + _22 * mat._23 + _23 * mat._33 + _24 * mat._43;
		f32 m24 = _21 * mat._14 + _22 * mat._24 + _23 * mat._34 + _24 * mat._44;

		f32 m31 = _31 * mat._11 + _32 * mat._21 + _33 * mat._31 + _34 * mat._41;
		f32 m32 = _31 * mat._12 + _32 * mat._22 + _33 * mat._32 + _34 * mat._42;
		f32 m33 = _31 * mat._13 + _32 * mat._23 + _33 * mat._33 + _34 * mat._43;
		f32 m34 = _31 * mat._14 + _32 * mat._24 + _33 * mat._34 + _34 * mat._44;

		f32 m41 = _41 * mat._11 + _42 * mat._21 + _43 * mat._31 + _44 * mat._41;
		f32 m42 = _41 * mat._12 + _42 * mat._22 + _43 * mat._32 + _44 * mat._42;
		f32 m43 = _41 * mat._13 + _42 * mat._23 + _43 * mat._33 + _44 * mat._43;
		f32 m44 = _41 * mat._14 + _42 * mat._24 + _43 * mat._34 + _44 * mat._44;

		return Matrix4(
			m11, m12, m13, m14,
			m21, m22, m23, m24,
			m31, m32, m33, m34,
			m41, m42, m43, m44);
	}

	Matrix4 getTranspose()
	{
		return Matrix4(
			_11, _21, _31, _41,
			_12, _22, _32, _42,
			_13, _23, _33, _43,
			_14, _24, _34, _44);
	}

	Matrix4 getInverse()
	{
		/* Calculate matrix inverse using Cramer's rule */
		Matrix4 src = getTranspose();
		f32 tmp[12];

		/* Calculate pairs for first 8 co-factors */
		tmp[0]  = src._33 * src._44, tmp[1]  = src._34 * src._43;
		tmp[2]  = src._32 * src._44, tmp[3]  = src._34 * src._42;
		tmp[4]  = src._32 * src._43, tmp[5]  = src._33 * src._42;
		tmp[6]  = src._31 * src._44, tmp[7]  = src._34 * src._41;
		tmp[8]  = src._31 * src._43, tmp[9]  = src._33 * src._41;
		tmp[10] = src._31 * src._42, tmp[11] = src._32 * src._41;

		/* Calculate first 8 co-factors */
		f32 m11 = tmp[0] * src._22 + tmp[3] * src._23 + tmp[4] * src._24;
		m11    -= tmp[1] * src._22 + tmp[2] * src._23 + tmp[5] * src._24;
		f32 m12 = tmp[1] * src._21 + tmp[6] * src._23 + tmp[9] * src._24;
		m12    -= tmp[0] * src._21 + tmp[7] * src._23 + tmp[8] * src._24;
		f32 m13 = tmp[2] * src._21 + tmp[7] * src._22 + tmp[10] * src._24;
		m13    -= tmp[3] * src._21 + tmp[6] * src._22 + tmp[11] * src._24;
		f32 m14 = tmp[5] * src._21 + tmp[8] * src._22 + tmp[11] * src._23;
		m14    -= tmp[4] * src._21 + tmp[9] * src._22 + tmp[10] * src._23;

		f32 m21 = tmp[1] * src._12 + tmp[2] * src._13 + tmp[5] * src._14;
		m21    -= tmp[0] * src._12 + tmp[3] * src._13 + tmp[4] * src._14;
		f32 m22 = tmp[0] * src._11 + tmp[7] * src._13 + tmp[8] * src._14;
		m22    -= tmp[1] * src._11 + tmp[6] * src._13 + tmp[9] * src._14;
		f32 m23 = tmp[3] * src._11 + tmp[6] * src._12 + tmp[11] * src._14;
		m23    -= tmp[2] * src._11 + tmp[7] * src._12 + tmp[10] * src._14;
		f32 m24 = tmp[4] * src._11 + tmp[9] * src._12 + tmp[10] * src._13;
		m24    -= tmp[5] * src._11 + tmp[8] * src._12 + tmp[11] * src._13;

		/* Calculate pairs for second 8 co-factors */
		tmp[0]  = src._13 * src._24, tmp[1]  = src._14 * src._23;
		tmp[2]  = src._12 * src._24, tmp[3]  = src._14 * src._22;
		tmp[4]  = src._12 * src._23, tmp[5]  = src._13 * src._22;
		tmp[6]  = src._11 * src._24, tmp[7]  = src._14 * src._21;
		tmp[8]  = src._11 * src._23, tmp[9]  = src._13 * src._21;
		tmp[10] = src._11 * src._22, tmp[11] = src._12 * src._21;

		/* Calculate second 8 co-factors */
		f32 m31 = tmp[0] * src._42 + tmp[3] * src._43 + tmp[4] * src._44;
		m31    -= tmp[1] * src._42 + tmp[2] * src._43 + tmp[5] * src._44;
		f32 m32 = tmp[1] * src._41 + tmp[6] * src._43 + tmp[9] * src._44;
		m32    -= tmp[0] * src._41 + tmp[7] * src._43 + tmp[8] * src._44;
		f32 m33 = tmp[2] * src._41 + tmp[7] * src._42 + tmp[10] * src._44;
		m33    -= tmp[3] * src._41 + tmp[6] * src._42 + tmp[11] * src._44;
		f32 m34 = tmp[5] * src._41 + tmp[8] * src._42 + tmp[11] * src._43;
		m34    -= tmp[4] * src._41 + tmp[9] * src._42 + tmp[10] * src._43;

		f32 m41 = tmp[1] * src._32 + tmp[2] * src._33 + tmp[5] * src._34;
		m41    -= tmp[0] * src._32 + tmp[3] * src._33 + tmp[4] * src._34;
		f32 m42 = tmp[0] * src._31 + tmp[7] * src._33 + tmp[8] * src._34;
		m42    -= tmp[1] * src._31 + tmp[6] * src._33 + tmp[9] * src._34;
		f32 m43 = tmp[3] * src._31 + tmp[6] * src._32 + tmp[11] * src._34;
		m43    -= tmp[2] * src._31 + tmp[7] * src._32 + tmp[10] * src._34;
		f32 m44 = tmp[4] * src._31 + tmp[9] * src._32 + tmp[10] * src._33;
		m44    -= tmp[5] * src._31 + tmp[8] * src._32 + tmp[11] * src._33;

		/* Calculate determinant */
		f32 det = src._11 * m11 + src._12 * m12 + src._13 * m13 + src._14 * m14;
		/* TODO */
		/*if (abs(det) < EPSILON)
			throw;*/
		det = 1.0f / det;

		/* Calculate final result */
		return Matrix4(
			m11 * det, m12 * det, m13 * det, m14 * det,
			m21 * det, m22 * det, m23 * det, m24 * det,
			m31 * det, m32 * det, m33 * det, m34 * det,
			m41 * det, m42 * det, m43 * det, m44 * det);
	}

	/* Matrix utilities */
	static Matrix4 identity()
	{
		return Matrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	static Matrix4 translation(f32 offsetX, f32 offsetY, f32 offsetZ)
	{
		return Matrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			offsetX, offsetY, offsetZ, 1);
	}

	static Matrix4 translation(const Vector3D &offset);

	static Matrix4 inverseTranslation(f32 offsetX, f32 offsetY, f32 offsetZ)
	{
		return Matrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-offsetX, -offsetY, -offsetZ, 1);
	}

	static Matrix4 inverseTranslation(const Vector3D &offset);

	static Matrix4 scale(f32 scaleX, f32 scaleY, f32 scaleZ)
	{
		return Matrix4(
			scaleX, 0, 0, 0,
			0, scaleY, 0, 0,
			0, 0, scaleZ, 0,
			0, 0, 0, 1);
	}

	static Matrix4 rotationX(f32 angle)
	{
		return Matrix4(
			1, 0, 0, 0,
			0, std::cos(angle), std::sin(angle), 0,
			0, -std::sin(angle), std::cos(angle), 0,
			0, 0, 0, 1);
	}

	static Matrix4 rotationY(f32 angle)
	{
		return Matrix4(
			std::cos(angle), 0, -std::sin(angle), 0,
			0, 1, 0, 0,
			std::sin(angle), 0, std::cos(angle), 0,
			0, 0, 0, 1);
	}

	static Matrix4 rotationZ(f32 angle)
	{
		return Matrix4(
			std::cos(angle), std::sin(angle), 0, 0,
			-std::sin(angle), std::cos(angle), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	static Matrix4 perspectiveFovLH(f32 fieldOfViewY, f32 aspectRatio, f32 znearPlane, f32 zfarPlane)
	{
		f32 h = std::cos(fieldOfViewY / 2) / std::sin(fieldOfViewY / 2);
		f32 w = h / aspectRatio;
		f32 q = zfarPlane / (zfarPlane - znearPlane);
		return Matrix4(
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, q, 1,
			0, 0, -q * znearPlane, 0);
	}

	static Matrix4 perspectiveFovRH(f32 fieldOfViewY, f32 aspectRatio, f32 znearPlane, f32 zfarPlane)
	{
		f32 h = std::cos(fieldOfViewY / 2) / std::sin(fieldOfViewY / 2);
		f32 w = h / aspectRatio;
		f32 q = zfarPlane / (znearPlane - zfarPlane);
		return Matrix4(
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, q, -1,
			0, 0, -q * znearPlane, 0);
	}

	static Matrix4 lookAtLH(const Vector3D &eye, const Vector3D &at, const Vector3D &up);
	static Matrix4 lookAtRH(const Vector3D &eye, const Vector3D &at, const Vector3D &up);
};
