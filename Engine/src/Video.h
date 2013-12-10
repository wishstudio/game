#pragma once

#include <string>

class Video
{
public:
	/* Create video device */
	static PVideo createVideo(u32 width, u32 height);
	virtual PDevice getDevice() const = 0;

	/* Resource management */
	virtual PTexture createTexture(u32 width, u32 height, const void *data) = 0;
	virtual PTexture createTexture(const std::string &path) = 0;

	virtual PVertexFormat createVertexFormat() = 0;
	virtual PVertexBuffer createVertexBuffer(PVertexFormat format, u32 size) = 0;
	virtual PIndexBuffer createIndexBuffer(VertexElementType type, u32 size) = 0;

	virtual PVertexShader createVertexShader(const char *program, const char *entrypoint) = 0;
	virtual PPixelShader createPixelShader(const char *program, const char *entrypoint) = 0;
	virtual PGeometryShader createGeometryShader(const char *program, const char *entrypoint) = 0;
	virtual PHullShader createHullShader(const char *program, const char *entrypoint) = 0;
	virtual PDomainShader createDomainShader(const char *program, const char *entrypoint) = 0;
	virtual PComputeShader createComputeShader(const char *program, const char *entrypoint) = 0;

	virtual PMaterial createMaterial() = 0;
	virtual PPass createPass() = 0;

	/* Render state manipulation */
	virtual void setViewport(s32 width, s32 height) = 0;
	virtual void setTexture(PTexture texture) = 0;
	virtual void setModelMatrix(const Matrix4 &matrix) = 0;
	virtual void setViewMatrix(const Matrix4 &matrix) = 0;
	virtual void setProjectionMatrix(const Matrix4 &matrix) = 0;
	virtual void setVertexShader(PVertexShader vertexShader) = 0;
	virtual void setPixelShader(PPixelShader pixelShader) = 0;
	virtual void setGeometryShader(PGeometryShader geometryShader) = 0;
	virtual void setHullShader(PHullShader hullShader) = 0;
	virtual void setDomainShader(PDomainShader domainShader) = 0;
	virtual void setComputeShader(PComputeShader computeShader) = 0;
	virtual void setMaterial(PMaterial material) = 0;

	/* Query states */
	virtual Vector2DI getBackBufferSize() const = 0;
	virtual u32 getVertexCount() const = 0;

	/* Rendering functions */
	virtual void beginDraw(Color clearColor) = 0;
	virtual void endDraw() = 0;

	virtual void draw(
		PVertexBuffer vertexBuffer,
		u32 startVertex,
		u32 count,
		PrimitiveTopology topology) = 0;
	virtual void drawIndexed(
		PVertexBuffer vertexBuffer,
		u32 startVertex,
		PIndexBuffer indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) = 0;
};
