#pragma once

#include <cstdint>
#include <memory>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

/* PI */
template <typename T>
struct pi
{
	operator T();
};

template <>
struct pi<float>
{
	operator float() { return 3.14159265359f; }
};

template <>
struct pi<double>
{
	operator double() { return 3.1415926535897932384626433832795028841971693993751; }
};

/* Epsilon related functions */
template <typename T>
struct epsilon
{
	operator T();
};

template <>
struct epsilon<float>
{
	operator float() { return 0.000001f; }
};

template <>
struct epsilon<double>
{
	operator double() { return 0.00000001; }
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

const float PI = pi<float>();
const double PI64 = pi<double>();
const float EPSILON = epsilon<float>();
const double EPSILON64 = epsilon<double>();

/* Enumerations */

enum PrimitiveTopology
{
	TOPOLOGY_POINTLIST,
	TOPOLOGY_LINELIST,
	TOPOLOGY_LINESTRIP,
	TOPOLOGY_TRIANGLELIST,
	TOPOLOGY_TRIANGLESTRIP,
	TOPOLOGY_COUNT
};

enum VertexElementSemantic
{
	SEMANTIC_POSITION,
	SEMANTIC_NORMAL,
	SEMANTIC_COLOR,
	SEMANTIC_TEXCOORD,
	SEMANTIC_COUNT
};

enum VertexElementType
{
	TYPE_BYTE, TYPE_BYTE1 = TYPE_BYTE, TYPE_BYTE2, TYPE_BYTE3, TYPE_BYTE4,
	TYPE_BYTE_NORM, TYPE_BYTE1_NORM = TYPE_BYTE_NORM, TYPE_BYTE2_NORM, TYPE_BYTE3_NORM, TYPE_BYTE4_NORM,
	TYPE_UBYTE, TYPE_UBYTE1 = TYPE_UBYTE, TYPE_UBYTE2, TYPE_UBYTE3, TYPE_UBYTE4,
	TYPE_UBYTE_NORM, TYPE_UBYTE1_NORM = TYPE_UBYTE_NORM, TYPE_UBYTE2_NORM, TYPE_UBYTE3_NORM, TYPE_UBYTE4_NORM,
	TYPE_SHORT, TYPE_SHORT1 = TYPE_SHORT, TYPE_SHORT2, TYPE_SHORT3, TYPE_SHORT4,
	TYPE_SHORT_NORM, TYPE_SHORT1_NORM = TYPE_SHORT_NORM, TYPE_SHORT2_NORM, TYPE_SHORT3_NORM, TYPE_SHORT4_NORM,
	TYPE_USHORT, TYPE_USHORT1 = TYPE_USHORT, TYPE_USHORT2, TYPE_USHORT3, TYPE_USHORT4,
	TYPE_USHORT_NORM, TYPE_USHORT1_NORM = TYPE_USHORT_NORM, TYPE_USHORT2_NORM, TYPE_USHORT3_NORM, TYPE_USHORT4_NORM,
	TYPE_INT, TYPE_INT1 = TYPE_INT, TYPE_INT2, TYPE_INT3, TYPE_INT4,
	TYPE_UINT, TYPE_UINT1 = TYPE_UINT, TYPE_UINT2, TYPE_UINT3, TYPE_UINT4,
	TYPE_FLOAT, TYPE_FLOAT1 = TYPE_FLOAT, TYPE_FLOAT2, TYPE_FLOAT3, TYPE_FLOAT4,
	TYPE_DOUBLE, TYPE_DOUBLE1 = TYPE_DOUBLE, TYPE_DOUBLE2, TYPE_DOUBLE3, TYPE_DOUBLE4,
	TYPE_COUNT
};

u32 getTypeSize(VertexElementType type);

enum FillMode
{
	FILLMODE_WIREFRAME,
	FILLMODE_SOLID
};

enum CullMode
{
	CULLMODE_NONE,
	CULLMODE_FRONT,
	CULLMODE_BACK
};

enum FrontMode
{
	FRONTMODE_CLOCKWISE,
	FRONTMODE_COUNTERCLOCKWISE
};

/* Forward declaration and typedefs */
class Device;
class Video;
class GUI;

class Texture;

class IndexBuffer;
class VertexBuffer;
class VertexFormat;

class VertexShader;
class PixelShader;
class GeometryShader;
class HullShader;
class DomainShader;
class ComputeShader;

class Material;
class Pass;

typedef std::shared_ptr<Device> PDevice;
typedef std::shared_ptr<Video> PVideo;
typedef std::shared_ptr<GUI> PGUI;

typedef std::shared_ptr<Texture> PTexture;

typedef std::shared_ptr<IndexBuffer> PIndexBuffer;
typedef std::shared_ptr<VertexBuffer> PVertexBuffer;
typedef std::shared_ptr<VertexFormat> PVertexFormat;

typedef std::shared_ptr<VertexShader> PVertexShader;
typedef std::shared_ptr<PixelShader> PPixelShader;
typedef std::shared_ptr<GeometryShader> PGeometryShader;
typedef std::shared_ptr<HullShader> PHullShader;
typedef std::shared_ptr<DomainShader> PDomainShader;
typedef std::shared_ptr<ComputeShader> PComputeShader;

typedef std::shared_ptr<Material> PMaterial;
typedef std::shared_ptr<Pass> PPass;
