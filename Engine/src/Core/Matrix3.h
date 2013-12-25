#pragma once

#include <cmath>
#include "Vector2D.h"

class Matrix3
{
public:
	union {
		struct {
			f32 _11, _12, _13;
			f32 _21, _22, _23;
			f32 _31, _32, _33;
		};
		f32 m[3][3];
	};

	Matrix3() {}
	Matrix3(
		f32 m11, f32 m12, f32 m13,
		f32 m21, f32 m22, f32 m23,
		f32 m31, f32 m32, f32 m33) :
		_11(m11), _12(m12), _13(m13),
		_21(m21), _22(m22), _23(m23),
		_31(m31), _32(m32), _33(m33) {}

	Matrix3(const Matrix3 &mat) = default;
	Matrix3 &operator= (const Matrix3 &mat) = default;

	Matrix3 operator* (const Matrix3 &mat) const
	{
		f32 m11 = _11 * mat._11 + _12 * mat._21 + _13 * mat._31;
		f32 m12 = _11 * mat._12 + _12 * mat._22 + _13 * mat._32;
		f32 m13 = _11 * mat._13 + _12 * mat._23 + _13 * mat._33;

		f32 m21 = _21 * mat._11 + _22 * mat._21 + _23 * mat._31;
		f32 m22 = _21 * mat._12 + _22 * mat._22 + _23 * mat._32;
		f32 m23 = _21 * mat._13 + _22 * mat._23 + _23 * mat._33;

		f32 m31 = _31 * mat._11 + _32 * mat._21 + _33 * mat._31;
		f32 m32 = _31 * mat._12 + _32 * mat._22 + _33 * mat._32;
		f32 m33 = _31 * mat._13 + _32 * mat._23 + _33 * mat._33;

		return Matrix3(
			m11, m12, m13,
			m21, m22, m23,
			m31, m32, m33);
	}

	friend Vector2D operator* (const Vector2D &vec, const Matrix3 &mat)
	{
		f32 x = vec.x * mat._11 + vec.y * mat._21 + mat._31;
		f32 y = vec.x * mat._12 + vec.y * mat._22 + mat._32;
		return Vector2D(x, y);
	}

	Matrix3 getTranspose()
	{
		return Matrix3(
			_11, _21, _31,
			_12, _22, _32,
			_13, _23, _33);
	}

	Matrix3 getInverse()
	{
		/* Calculate matrix inverse using Cramer's rule */
		Matrix3 src = getTranspose();

		/* Calculate co-factors */
		f32 m11 = src._22 * src._33 - src._23 * src._32;
		f32 m12 = src._23 * src._31 - src._21 * src._33;
		f32 m13 = src._21 * src._32 - src._22 * src._31;

		f32 m21 = src._32 * src._13 - src._33 * src._12;
		f32 m22 = src._33 * src._11 - src._31 * src._13;
		f32 m23 = src._31 * src._12 - src._32 * src._11;

		f32 m31 = src._12 * src._23 - src._22 * src._13;
		f32 m32 = src._13 * src._21 - src._23 * src._11;
		f32 m33 = src._11 * src._22 - src._12 * src._21;

		/* Calculate determinant */
		f32 det = src._11 * m11 + src._12 * m12 + src._13 * m13;
		/* TODO */
		/*if (isZero(det))
			throw;*/
		det = 1.0f / det;

		return Matrix3(
			m11 * det, m12 * det, m13 * det,
			m21 * det, m22 * det, m23 * det,
			m31 * det, m32 * det, m33 * det);
	}

	/* Matrix utilities */
	static Matrix3 identity()
	{
		return Matrix3(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1);
	}

	static Matrix3 translation(f32 offsetX, f32 offsetY)
	{
		return Matrix3(
			1, 0, 0,
			0, 1, 0,
			offsetX, offsetY, 1);
	}

	static Matrix3 translation(const Vector2D &offset);

	static Matrix3 inverseTranslation(f32 offsetX, f32 offsetY)
	{
		return Matrix3(
			1, 0, 0,
			0, 1, 0,
			-offsetX, -offsetY, 1);
	}

	static Matrix3 inverseTranslation(const Vector2D &offset);

	static Matrix3 scale(f32 scaleX, f32 scaleY)
	{
		return Matrix3(
			scaleX, 0, 0,
			0, scaleY, 0,
			0, 0, 1);
	}

	/* Rotation counter-clockwise */
	static Matrix3 rotation(f32 angle)
	{
		return Matrix3(
			std::cos(angle), std::sin(angle), 0,
			-std::sin(angle), std::cos(angle), 0,
			0, 0, 1);
	}
};