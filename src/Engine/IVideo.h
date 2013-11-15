#pragma once

#include <string>
#include "Vector3D.h"
#include "Matrix4.h"

class IComputeShader;
class IDomainShader;
class IGeometryShader;
class IHullShader;
class IIndexBuffer;
class IPixelShader;
class ITexture;
class IVertexBuffer;
class IVertexShader;
class IVideo
{
public:
	/* Resource management */
	virtual ITexture *createTexture(int width, int height, const void *data) = 0;
	virtual ITexture *createTexture(const std::string &path) = 0;
	virtual void deleteTexture(ITexture *texture) = 0;

	virtual IVertexFormat *createVertexFormat() = 0;
	virtual IVertexBuffer *createVertexBuffer(IVertexFormat *format, u32 size) = 0;
	virtual IIndexBuffer *createIndexBuffer(VertexElementType type, u32 size) = 0;

	virtual IVertexShader *createVertexShader(const char *program, const char *entrypoint) = 0;
	virtual IPixelShader *createPixelShader(const char *program, const char *entrypoint) = 0;
	virtual IGeometryShader *createGeometryShader(const char *program, const char *entrypoint) = 0;
	virtual IHullShader *createHullShader(const char *program, const char *entrypoint) = 0;
	virtual IDomainShader *createDomainShader(const char *program, const char *entrypoint) = 0;
	virtual IComputeShader *createComputeShader(const char *program, const char *entrypoint) = 0;

	/* Render state manipulation */
	virtual void setViewport(s32 width, s32 height) = 0;
	virtual void setTexture(ITexture *texture) = 0;
	virtual void setModelMatrix(const Matrix4 &matrix) = 0;
	virtual void setViewMatrix(const Matrix4 &matrix) = 0;
	virtual void setProjectionMatrix(const Matrix4 &matrix) = 0;
	virtual void setVertexShader(IVertexShader *shader) = 0;
	virtual void setPixelShader(IPixelShader *shader) = 0;
	virtual void setGeometryShader(IGeometryShader *shader) = 0;
	virtual void setHullShader(IHullShader *shader) = 0;
	virtual void setDomainShader(IDomainShader *shader) = 0;
	virtual void setComputeShader(IComputeShader *shader) = 0;

	/* Query functions */
	virtual u32 getVertexCount() const = 0;

	/* Rendering functions */
	virtual void clearScreen() = 0;

	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	virtual void draw(IVertexBuffer *vertexBuffer, u32 startVertex, u32 count, PrimitiveTopology topology) = 0;
	virtual void drawIndexed(
		IVertexBuffer *vertexBuffer,
		u32 startVertex,
		IIndexBuffer *indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) = 0;
};
