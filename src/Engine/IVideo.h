#pragma once

#include <string>

class MeshBuffer;
class ITTexture;
class IVideo
{
public:
	virtual void clearScreen() = 0;
	virtual void setViewport(s32 width, s32 height) = 0;

	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	virtual ITTexture *createTexture(const std::string &path) = 0;
	virtual void deleteTexture(ITTexture *texture) = 0;

	virtual MeshBuffer *createMeshBuffer() = 0;
	virtual void deleteMeshBuffer(MeshBuffer *buffer) = 0;

	virtual void setTexture(ITTexture *texture) = 0;
	virtual void setWorldMatrix(const matrix4 &matrix) = 0;
	virtual void setViewMatrix(const matrix4 &matrix) = 0;
	virtual void setProjectionMatrix(const matrix4 &matrix) = 0;

	virtual void drawMeshBuffer(MeshBuffer *buffer) = 0;
};
