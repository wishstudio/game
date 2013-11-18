#pragma once

#include "../Color.h"
#include "../Video.h"

#include <d3d11_2.h>
#include <wincodec.h>

class D3D11VertexShader;
class Win32Device;
class D3D11Video: public Video
{
public:
	D3D11Video();
	virtual ~D3D11Video();

	bool init(Win32Device *device);
	void resetBackBuffer();

	virtual PTexture createTexture(u32 width, u32 height, const void *data) override;
	virtual PTexture createTexture(const std::string &path) override;

	virtual PVertexFormat createVertexFormat() override;
	virtual PVertexBuffer createVertexBuffer(PVertexFormat format, u32 size) override;
	virtual PIndexBuffer createIndexBuffer(VertexElementType type, u32 size) override;

	virtual PVertexShader createVertexShader(const char *program, const char *entrypoint) override;
	virtual PPixelShader createPixelShader(const char *program, const char *entrypoint) override;
	virtual PGeometryShader createGeometryShader(const char *program, const char *entrypoint) override;
	virtual PHullShader createHullShader(const char *program, const char *entrypoint) override;
	virtual PDomainShader createDomainShader(const char *program, const char *entrypoint) override;
	virtual PComputeShader createComputeShader(const char *program, const char *entrypoint) override;

	virtual void setViewport(s32 width, s32 height) override;
	virtual void setTexture(PTexture texture) override;
	virtual void setModelMatrix(const Matrix4 &matrix) override;
	virtual void setViewMatrix(const Matrix4 &matrix) override;
	virtual void setProjectionMatrix(const Matrix4 &matrix) override;
	virtual void setVertexShader(PVertexShader vertexShader) override;
	virtual void setPixelShader(PPixelShader pixelShader) override;
	virtual void setGeometryShader(PGeometryShader geometryShader) override;
	virtual void setHullShader(PHullShader hullShader) override;
	virtual void setDomainShader(PDomainShader domainShader) override;
	virtual void setComputeShader(PComputeShader computeShader) override;

	virtual u32 getVertexCount() const override { return vertexCount; }

	virtual void beginDraw(Color clearColor) override;
	virtual void endDraw() override;

	virtual void draw(PVertexBuffer vertexBuffer, u32 startVertex, u32 count, PrimitiveTopology topology) override;
	virtual void drawIndexed(
		PVertexBuffer vertexBuffer,
		u32 startVertex,
		PIndexBuffer indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) override;

	/* INTERNAL INTERFACE */
	ID3D11Device *getDevice() const { return pDevice; };
	ID3D11DeviceContext *getDeviceContext() const { return pContext; }

private:
	ID3DBlob *createShader(const char *program, const char *entrypoint, const char *target);
	char *decodeImage(const char *raw, u32 size, u32 *width, u32 *height);
	char *getResourceData(const char *resourceName, u32 *fileSize);
	PTexture createTexture(u32 width, u32 height, const void *initialData, D3D11_USAGE usage, UINT bindFlag);

	u32 vertexCount;

	IWICImagingFactory *pFactory;
	
	D3D_FEATURE_LEVEL featureLevel;

	ID3D11Device *pDevice = nullptr;
	ID3D11DeviceContext *pContext = nullptr;
	IDXGISwapChain *pSwapChain = nullptr;
	ID3D11Texture2D *pDepthStencil = nullptr;
	ID3D11DepthStencilView *pDepthStencilView = nullptr;
	ID3D11DepthStencilState *pDepthStencilState = nullptr;
	ID3D11RenderTargetView *pBackBufferRenderTargetView = nullptr;
	ID3D11RasterizerState *pRasterizerState = nullptr;
	
	ID3D11Buffer *pMatrixBuffer = nullptr;
	Matrix4 modelMatrix, viewMatrix, projectionMatrix;

	std::weak_ptr<D3D11VertexShader> currentVertexShader;
};
