#pragma once

#include <string>
#include "Vector3D.h"
#include "Matrix4.h"

class MeshBuffer;
class ITexture;
class IVideo
{
public:
	virtual void clearScreen() = 0;
	virtual void setViewport(s32 width, s32 height) = 0;

	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	virtual ITexture *createTexture(const std::string &path) = 0;
	virtual void deleteTexture(ITexture *texture) = 0;

	virtual MeshBuffer *createMeshBuffer() = 0;
	virtual void deleteMeshBuffer(MeshBuffer *buffer) = 0;

	virtual void setTexture(ITexture *texture) = 0;
	virtual void setModelMatrix(const Matrix4 &matrix) = 0;
	virtual void setViewMatrix(const Matrix4 &matrix) = 0;
	virtual void setProjectionMatrix(const Matrix4 &matrix) = 0;

	virtual void drawMeshBuffer(MeshBuffer *buffer) = 0;
};
