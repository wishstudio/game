#pragma once

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

/* PI */
template <typename T>
struct pi
{
	T operator ();
};

template <>
struct pi<f32>
{
	operator f32() { return 3.14159265359f; }
};

template <>
struct pi<f64>
{
	operator f64() { return 3.1415926535897932384626433832795028841971693993751; }
};

/* Epsilon related functions */
template <typename T>
struct epsilon
{
	T operator ();
};

template <>
struct epsilon<f32>
{
	operator f32() { return 0.000001f; }
};

template <>
struct epsilon<f64>
{
	operator f64() { return 0.00000001; }
};

template <typename T>
bool isEqual(T x, T y)
{
	return x > y - epsilon<T>() && x < y + epsilon<T>();
}

template <typename T>
bool isZero(T x)
{
	return isEqual<T>(x, 0);
}

const f32 PI = pi<f32>();
const f64 PI64 = pi<f64>();
const f32 EPSILON = epsilon<f32>();
const f64 EPSILON64 = epsilon<f64>();
