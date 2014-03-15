#pragma once

#include <Video.h>
#include <d3d11_2.h>
#include <wincodec.h>

class D3D11Backend;
class D3D11VertexBuffer;
class D3D11VertexShader;
class D3D11PixelShader;
class D3D11GeometryShader;
class D3D11HullShader;
class D3D11DomainShader;
class D3D11ComputeShader;
class Win32Device;
class D3D11Video: public Video
{
public:
	D3D11Video();
	virtual ~D3D11Video();

	bool init(std::shared_ptr<Win32Device> device);
	void resetBackBuffer();

	virtual PDevice getDevice() const override;

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

	virtual PMaterial createMaterial() override;
	virtual PPass createPass() override;

	virtual void setViewport(s32 width, s32 height) override;
	virtual void setTexture(PTexture texture) override;
	virtual void setMaterial(PMaterial material) override;

	virtual void setVertexShader(PVertexShader vertexShader) override;
	virtual void setPixelShader(PPixelShader pixelShader) override;
	virtual void setGeometryShader(PGeometryShader geometryShader) override;
	virtual void setHullShader(PHullShader hullShader) override;
	virtual void setDomainShader(PDomainShader domainShader) override;
	virtual void setComputeShader(PComputeShader computeShader) override;

	virtual PVertexShader getVertexShader() const override;
	virtual PPixelShader getPixelShader() const override;
	virtual PGeometryShader getGeometryShader() const override;
	virtual PHullShader getHullShader() const override;
	virtual PDomainShader getDomainShader() const override;
	virtual PComputeShader getComputeShader() const override;

	virtual Vector2DI getBackBufferSize() const override;
	virtual u32 getVertexCount() const override { return vertexCount; }

	virtual void beginDraw(Color clearColor) override;
	virtual void endDraw() override;

	virtual void draw(
		PVertexBuffer vertexBuffer,
		u32 startVertex,
		u32 count,
		PrimitiveTopology topology) override;
	virtual void drawIndexed(
		PVertexBuffer vertexBuffer,
		u32 startVertex,
		PIndexBuffer indexBuffer,
		u32 startIndex,
		u32 count,
		PrimitiveTopology topology
		) override;

	/* INTERNAL INTERFACE */
	ID3D11Device *getD3D11Device() const { return pDevice; };
	ID3D11DeviceContext *getD3D11DeviceContext() const { return pContext; }

private:
	ID3DBlob *createShader(const char *program, const char *entrypoint, const char *target, D3D11Backend **shaderReflection);
	char *decodeImage(const char *raw, u32 size, u32 *width, u32 *height);
	char *getResourceData(const char *resourceName, u32 *fileSize);
	PTexture createTexture(u32 width, u32 height, const void *initialData, D3D11_USAGE usage, UINT bindFlag);
	void prepareDraw(D3D11VertexBuffer *vertexBuffer);

	u32 vertexCount;
	u32 width, height;

	IWICImagingFactory *pFactory;
	
	D3D_FEATURE_LEVEL featureLevel;

	std::shared_ptr<Win32Device> device;

	ID3D11Device *pDevice = nullptr;
	ID3D11DeviceContext *pContext = nullptr;
	IDXGISwapChain *pSwapChain = nullptr;
	ID3D11Texture2D *pDepthStencil = nullptr;
	ID3D11DepthStencilView *pDepthStencilView = nullptr;
	ID3D11DepthStencilState *pDepthStencilState = nullptr;
	ID3D11RenderTargetView *pBackBufferRenderTargetView = nullptr;
	ID3D11RasterizerState *pRasterizerState = nullptr;
	ID3D11BlendState *pBlendState = nullptr;

	std::weak_ptr<D3D11VertexShader> vertexShader;
	std::weak_ptr<D3D11PixelShader> pixelShader;
	std::weak_ptr<D3D11GeometryShader> geometryShader;
	std::weak_ptr<D3D11HullShader> hullShader;
	std::weak_ptr<D3D11DomainShader> domainShader;
	std::weak_ptr<D3D11ComputeShader> computeShader;
};
