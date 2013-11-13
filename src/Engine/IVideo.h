#pragma once

#include <string>
#include "Vector3D.h"
#include "Matrix4.h"

class IIndexBuffer;
class ITexture;
class IVertexBuffer;
class IVideo
{
public:
	/* Resource management */
	virtual ITexture *createTexture(const std::string &path) = 0;
	virtual void deleteTexture(ITexture *texture) = 0;

	virtual IVertexFormat *createVertexFormat() = 0;
	virtual IVertexBuffer *createVertexBuffer(IVertexFormat *format, u32 size) = 0;
	virtual IIndexBuffer *createIndexBuffer(VertexElementType type, u32 size) = 0;

	/* Render state manipulation */
	virtual void setTexture(ITexture *texture) = 0;
	virtual void setModelMatrix(const Matrix4 &matrix) = 0;
	virtual void setViewMatrix(const Matrix4 &matrix) = 0;
	virtual void setProjectionMatrix(const Matrix4 &matrix) = 0;

	/* Rendering functions */
	virtual void clearScreen() = 0;
	virtual void setViewport(s32 width, s32 height) = 0;

	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	virtual void drawIndexed(
		IVertexBuffer *vertexBuffer,
		u32 startVertex,
		IIndexBuffer *indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) = 0;
};
