#pragma once

#include "../Color.h"
#include "../IVideo.h"

#include <d3d11_2.h>
#include <wincodec.h>

struct Vertex
{
	vector3df pos;
	Color color;
	f32 u, v;
};

struct MatrixBuffer
{
	matrix4 world;
	matrix4 view;
	matrix4 projection;
};

class MeshBuffer
{
public:
	std::vector<Vertex> vertexBuffer;
	std::vector<u32> indexBuffer;

	bool vertexDirty = true, indexDirty = true;
	ID3D11Buffer *pVertexBuffer = nullptr, *pIndexBuffer = nullptr;
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

	virtual void setViewport(s32 width, s32 height) override;
	virtual void clearScreen() override;

	virtual void beginDraw() override;
	virtual void endDraw() override;

	virtual ITTexture *createTexture(const std::string &path) override;
	virtual void deleteTexture(ITTexture *texture) override;

	virtual MeshBuffer *createMeshBuffer() override;
	virtual void deleteMeshBuffer(MeshBuffer *buffer) override;

	virtual void setTexture(ITTexture *texture) override;
	virtual void setWorldMatrix(const matrix4 &matrix) override;
	virtual void setViewMatrix(const matrix4 &matrix) override;
	virtual void setProjectionMatrix(const matrix4 &matrix) override;

	virtual void drawMeshBuffer(MeshBuffer *buffer) override;

private:
	void createShaders();
	char *decodeImage(const char *raw, int size, int *width, int *height);
	char *getResourceData(const char *resourceName, int *fileSize);
	ITTexture *createTexture(int width, int height, const void *initialData, D3D11_USAGE usage, UINT bindFlag);
	void updateHardwareBuffer(MeshBuffer *buffer);

	IWICImagingFactory *pFactory;
	
	D3D_FEATURE_LEVEL featureLevel;

	ID3D11Device *pDevice = nullptr;
	ID3D11DeviceContext *pContext = nullptr;
	IDXGISwapChain *pSwapChain = nullptr;
	ID3D11Texture2D *pDepthStencil = nullptr;
	ID3D11DepthStencilView *pDepthStencilView = nullptr;
	ID3D11DepthStencilState *pDepthStencilState = nullptr;
	ID3D11RenderTargetView *pBackBufferRenderTargetView = nullptr;

	ID3D11InputLayout *pInputLayout = nullptr;
	ID3D11VertexShader *pVertexShader = nullptr;
	ID3D11PixelShader *pPixelShader = nullptr;
	
	ID3D11Buffer *pMatrixBuffer = nullptr;
	MatrixBuffer matrixBuffer;

	ID3D11SamplerState *pSamplerState = nullptr;
};
