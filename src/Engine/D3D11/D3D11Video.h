#pragma once

#include "../Color.h"
#include "../IVideo.h"

#include <d3d11_2.h>
#include <wincodec.h>

struct MatrixBuffer
{
	Matrix4 model;
	Matrix4 view;
	Matrix4 projection;
};

class ITexture;
class Win32WindowSystem;
class D3D11Video: public IVideo
{
public:
	D3D11Video();
	virtual ~D3D11Video();

	bool init(Win32WindowSystem *windowSystem);
	void resetBackBuffer();

	virtual ITexture *createTexture(const std::string &path) override;
	virtual void deleteTexture(ITexture *texture) override;

	virtual IVertexFormat *createVertexFormat() override;
	virtual IVertexBuffer *createVertexBuffer(IVertexFormat *format, u32 size) override;
	virtual IIndexBuffer *createIndexBuffer(VertexElementType type, u32 size) override;

	virtual void setTexture(ITexture *texture) override;
	virtual void setModelMatrix(const Matrix4 &matrix) override;
	virtual void setViewMatrix(const Matrix4 &matrix) override;
	virtual void setProjectionMatrix(const Matrix4 &matrix) override;

	virtual void setViewport(s32 width, s32 height) override;
	virtual void clearScreen() override;

	virtual void beginDraw() override;
	virtual void endDraw() override;

	virtual void drawIndexed(
		IVertexBuffer *vertexBuffer,
		u32 startVertex,
		IIndexBuffer *indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) override;

	/* INTERNAL INTERFACE */
	ID3D11Device *getDevice() const { return pDevice; };
	ID3D11DeviceContext *getDeviceContext() const { return pContext; }

private:
	void createShaders();
	char *decodeImage(const char *raw, int size, int *width, int *height);
	char *getResourceData(const char *resourceName, int *fileSize);
	ITexture *createTexture(int width, int height, const void *initialData, D3D11_USAGE usage, UINT bindFlag);

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

	ID3DBlob *pVertexShaderData = nullptr, *pPixelShaderData = nullptr;
	ID3D11VertexShader *pVertexShader = nullptr;
	ID3D11PixelShader *pPixelShader = nullptr;
	
	ID3D11Buffer *pMatrixBuffer = nullptr;
	MatrixBuffer matrixBuffer;
};
