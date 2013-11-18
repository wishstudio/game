#pragma once

#include "../Color.h"
#include "../IVideo.h"

#include <d3d11_2.h>
#include <wincodec.h>

class ITexture;
class Win32Device;
class D3D11Shader;
class D3D11Video: public IVideo
{
public:
	D3D11Video();
	virtual ~D3D11Video();

	bool init(Win32Device *device);
	void resetBackBuffer();

	virtual ITexture *createTexture(int width, int height, const void *data) override;
	virtual ITexture *createTexture(const std::string &path) override;
	virtual void deleteTexture(ITexture *texture) override;

	virtual IVertexFormat *createVertexFormat() override;
	virtual IVertexBuffer *createVertexBuffer(IVertexFormat *format, u32 size) override;
	virtual IIndexBuffer *createIndexBuffer(VertexElementType type, u32 size) override;

	virtual IVertexShader *createVertexShader(const char *program, const char *entrypoint) override;
	virtual IPixelShader *createPixelShader(const char *program, const char *entrypoint) override;
	virtual IGeometryShader *createGeometryShader(const char *program, const char *entrypoint) override;
	virtual IHullShader *createHullShader(const char *program, const char *entrypoint) override;
	virtual IDomainShader *createDomainShader(const char *program, const char *entrypoint) override;
	virtual IComputeShader *createComputeShader(const char *program, const char *entrypoint) override;

	virtual void setViewport(s32 width, s32 height) override;
	virtual void setTexture(ITexture *texture) override;
	virtual void setModelMatrix(const Matrix4 &matrix) override;
	virtual void setViewMatrix(const Matrix4 &matrix) override;
	virtual void setProjectionMatrix(const Matrix4 &matrix) override;
	virtual void setVertexShader(IVertexShader *shader) override;
	virtual void setPixelShader(IPixelShader *shader) override;
	virtual void setGeometryShader(IGeometryShader *shader) override;
	virtual void setHullShader(IHullShader *shader) override;
	virtual void setDomainShader(IDomainShader *shader) override;
	virtual void setComputeShader(IComputeShader *shader) override;

	virtual u32 getVertexCount() const override { return vertexCount; }

	virtual void beginDraw(Color clearColor) override;
	virtual void endDraw() override;

	virtual void draw(IVertexBuffer *vertexBuffer, u32 startVertex, u32 count, PrimitiveTopology topology) override;
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
	ID3DBlob *createShader(const char *program, const char *entrypoint, const char *target);
	char *decodeImage(const char *raw, int size, int *width, int *height);
	char *getResourceData(const char *resourceName, int *fileSize);
	ITexture *createTexture(int width, int height, const void *initialData, D3D11_USAGE usage, UINT bindFlag);

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
	D3D11Shader *currentVertexShader = nullptr;
};
