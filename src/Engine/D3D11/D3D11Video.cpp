#include <stdafx.h>

#include <d3dcompiler.h>
#include "../Device/Win32Device.h"
#include "D3D11IndexBuffer.h"
#include "D3D11Pass.h"
#include "D3D11Shader.h"
#include "D3D11Texture.h"
#include "D3D11VertexBuffer.h"
#include "D3D11VertexFormat.h"
#include "D3D11Video.h"

typedef HRESULT(_stdcall * PWICCBS)(REFWICPixelFormatGUID, IWICBitmapSource *, IWICBitmapSource **ppIDst);
static PWICCBS _WICConvertBitmapSource;

D3D11Video::D3D11Video()
{
	pFactory = NULL;

	CoInitialize(NULL);
	/* CLSID_WICImagingFactory(2) won't work on XP */
	CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID *)&pFactory);

	/* Find WICConvertBitmapSource function */
	HMODULE hm = LoadLibraryA("WindowsCodecs.dll");
	_WICConvertBitmapSource = (PWICCBS) GetProcAddress(hm, "WICConvertBitmapSource");
}

D3D11Video::~D3D11Video()
{
	if (pFactory)
		pFactory->Release();
	CoUninitialize();

	if (pDevice)
		pDevice->Release();
	if (pContext)
		pContext->Release();
	if (pSwapChain)
		pSwapChain->Release();
	if (pDepthStencil)
		pDepthStencil->Release();
	if (pDepthStencilView)
		pDepthStencilView->Release();
	if (pDepthStencilState)
		pDepthStencilState->Release();
	if (pBackBufferRenderTargetView)
		pBackBufferRenderTargetView->Release();
	if (pRasterizerState)
		pRasterizerState->Release();
	if (pBlendState)
		pBlendState->Release();
	
	if (pMatrixBuffer)
		pMatrixBuffer->Release();
}

bool D3D11Video::init(Win32Device *device)
{
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	device->getWindowSize(&width, &height);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof swapChainDesc);
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = device->getWindowHandle();
	swapChainDesc.Windowed = (width != 0) || (height != 0);
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	HMODULE hm = LoadLibraryA("d3d11.dll");
	if (hm == nullptr)
		return false;
	typedef HRESULT(__stdcall *PCDASC)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT,
		DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
	PCDASC D3D11CreateDeviceAndSwapChain = (PCDASC)GetProcAddress(hm, "D3D11CreateDeviceAndSwapChain");
	if (D3D11CreateDeviceAndSwapChain == nullptr)
		return false;

	HRESULT hr;
#ifdef _DEBUG
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = 0;
#endif
	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, &featureLevel, &pContext);
	if (FAILED(hr))
		return false;

	/* Matrix buffer */
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(Matrix4);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	hr = pDevice->CreateBuffer(&desc, NULL, &pMatrixBuffer);
	if (FAILED(hr))
		return false;

	/* Depth stencil buffer */
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT; /* TODO */
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &pDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT; /* TODO */
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.Flags = 0;
	pDevice->CreateDepthStencilView(pDepthStencil, &depthStencilViewDesc, &pDepthStencilView);

	/* Depth stencil state */
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilStateDesc.StencilEnable = false;
	pDevice->CreateDepthStencilState(&depthStencilStateDesc, &pDepthStencilState);
	if (FAILED(hr))
		return false;
	pContext->OMSetDepthStencilState(pDepthStencilState, 1);

	/* Rasterizer state */
	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FrontCounterClockwise = true;
	rasterizerStateDesc.DepthBias = 0;
	rasterizerStateDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerStateDesc.DepthBiasClamp = 0;
	rasterizerStateDesc.DepthClipEnable = true;
	rasterizerStateDesc.ScissorEnable = false;
	rasterizerStateDesc.MultisampleEnable = false;
	rasterizerStateDesc.AntialiasedLineEnable = true;
	pDevice->CreateRasterizerState(&rasterizerStateDesc, &pRasterizerState);
	pContext->RSSetState(pRasterizerState);

	/* Blend state */
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof blendDesc);
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	pDevice->CreateBlendState(&blendDesc, &pBlendState);
	f32 blendFactor[4];
	pContext->OMSetBlendState(pBlendState, blendFactor, 0xFFFFFFFF);

	/* Set viewport */
	setViewport(width, height);

	/* Set render target */
	resetBackBuffer();

	return true;
}

void D3D11Video::resetBackBuffer()
{
	ID3D11Texture2D *backBufferTexture;
	pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **) &backBufferTexture);
	pDevice->CreateRenderTargetView(backBufferTexture, 0, &pBackBufferRenderTargetView);
	backBufferTexture->Release();
	pContext->OMSetRenderTargets(1, &pBackBufferRenderTargetView, pDepthStencilView);
}

char *D3D11Video::decodeImage(const char *raw, u32 size, u32 *width, u32 *height)
{
	IWICStream *pStream = nullptr;
	IWICBitmapDecoder *pDecoder = nullptr;
	IWICBitmapFrameDecode *pFrame = nullptr;
	IWICBitmapSource *pSource = nullptr;
	char *buffer = nullptr;
	HRESULT hr;

	hr = pFactory->CreateStream(&pStream);
	if (FAILED(hr))
		goto Finish;

	hr = pStream->InitializeFromMemory((WICInProcPointer)raw, size);
	if (FAILED(hr))
		goto Finish;

	hr = pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr))
		goto Finish;

	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr))
		goto Finish;

	UINT w, h;
	hr = pFrame->GetSize(&w, &h);
	if (FAILED(hr))
		goto Finish;
	*width = w;
	*height = h;

	WICPixelFormatGUID formatGUID;
	hr = pFrame->GetPixelFormat(&formatGUID);
	if (FAILED(hr))
		goto Finish;

	pSource = pFrame;
	/* Check if it is already 32bpp RGBA */
	if (!IsEqualGUID(formatGUID, GUID_WICPixelFormat32bppBGRA))
	{
		/* Need to convert */
		IWICBitmapSource *pConverter;
		hr = _WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, pSource, &pConverter);
		if (FAILED(hr))
			goto Finish;
		pSource->Release();
		pSource = pConverter;
	}
	UINT cbStride = w * 4;
	UINT cbBufferSize = cbStride * h;
	buffer = (char *) malloc(cbBufferSize);
	hr = pSource->CopyPixels(nullptr, cbStride, cbBufferSize, (BYTE *)buffer);
	if (FAILED(hr))
	{
		free(buffer);
		buffer = nullptr;
	}

Finish:
	if (pSource)
		pSource->Release();
	if (pDecoder)
		pDecoder->Release();
	if (pStream)
		pStream->Release();
	return buffer;
}

char *D3D11Video::getResourceData(const char *resourceName, u32 *fileSize)
{
	HANDLE hFile = CreateFileA(resourceName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return nullptr;
	LARGE_INTEGER size;
	if (GetFileSizeEx(hFile, &size) == 0)
	{
		CloseHandle(hFile);
		return NULL;
	}
	(*fileSize) = (u32)size.QuadPart;
	char *content = (char *)malloc(*fileSize);
	DWORD bytesRead; /* XP Requires this */
	if (!ReadFile(hFile, content, (*fileSize), &bytesRead, NULL))
	{
		free(content);
		content = NULL;
	}
	CloseHandle(hFile);
	return content;
}

PTexture D3D11Video::createTexture(u32 width, u32 height, const void *initialData, D3D11_USAGE usage, UINT bindFlag)
{
	ID3D11Texture2D *pTexture2D;
	ID3D11ShaderResourceView *pShaderResourceView;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = usage;
	textureDesc.BindFlags = bindFlag;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (usage == D3D11_USAGE_DYNAMIC)
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (initialData)
	{
		D3D11_SUBRESOURCE_DATA dataDesc;
		dataDesc.pSysMem = initialData;
		dataDesc.SysMemPitch = width * 4;
		dataDesc.SysMemSlicePitch = 0;
		pDevice->CreateTexture2D(&textureDesc, &dataDesc, &pTexture2D);
	}
	else
		pDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	resourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = 1;

	pDevice->CreateShaderResourceView(pTexture2D, &resourceViewDesc, &pShaderResourceView);

	return std::make_shared<D3D11Texture>(pTexture2D, pShaderResourceView, width, height);
};

PTexture D3D11Video::createTexture(u32 width, u32 height, const void *data)
{
	return createTexture(width, height, data, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE);
}

PTexture D3D11Video::createTexture(const std::string &path)
{
	u32 size;
	std::unique_ptr<char> raw(getResourceData(path.c_str(), &size));
	if (raw == nullptr)
		return nullptr;
	u32 width, height;
	std::unique_ptr<char> image(decodeImage(raw.get(), size, &width, &height));
	if (image == nullptr)
		return nullptr;
	return createTexture(width, height, image.get(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE);
}

PVertexFormat D3D11Video::createVertexFormat()
{
	return std::make_shared<D3D11VertexFormat>();
}

PVertexBuffer D3D11Video::createVertexBuffer(PVertexFormat format, u32 size)
{
	return std::make_shared<D3D11VertexBuffer>(this, format, size);
}

PIndexBuffer D3D11Video::createIndexBuffer(VertexElementType type, u32 size)
{
	return std::make_shared<D3D11IndexBuffer>(this, type, size);
}

ID3DBlob *D3D11Video::createShader(const char *program, const char *entrypoint, const char *target)
{
	typedef HRESULT(_stdcall * PD3DCOMPILE)(
		LPCVOID pSrcData,
		SIZE_T SrcDataSize,
		LPCSTR pSourceName,
		const D3D_SHADER_MACRO *pDefines,
		ID3DInclude *pInclude,
		LPCSTR pEntrypoint,
		LPCSTR pTarget,
		UINT Flags1,
		UINT Flags2,
		ID3DBlob **ppCode,
		ID3DBlob **ppErrorMsgs
		);
	HMODULE hm = LoadLibraryA("D3DCompiler_43.dll");
	PD3DCOMPILE D3DCompile = (PD3DCOMPILE)GetProcAddress(hm, "D3DCompile");

	ID3DBlob *shaderBlob, *errMsg;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;
	D3DCompile(program, strlen(program), nullptr, nullptr, nullptr, entrypoint, target, compileFlags, 0, &shaderBlob, &errMsg);

	if (errMsg)
	{
		printf("Shader compilation error. Message:\n%s\n", errMsg->GetBufferPointer());
		errMsg->Release();
		return nullptr;
	}
	return shaderBlob;
}

PVertexShader D3D11Video::createVertexShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "vs_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11VertexShader *pVertexShader = nullptr;
	pDevice->CreateVertexShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pVertexShader);
	return std::make_shared<D3D11VertexShader>(pVertexShader, pBytecode);
}

PPixelShader D3D11Video::createPixelShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "ps_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11PixelShader *pPixelShader = nullptr;
	pDevice->CreatePixelShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pPixelShader);
	return std::make_shared<D3D11PixelShader>(pPixelShader);
}

PGeometryShader D3D11Video::createGeometryShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "gs_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11GeometryShader *pGeometryShader = nullptr;
	pDevice->CreateGeometryShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pGeometryShader);
	return std::make_shared<D3D11GeometryShader>(pGeometryShader);
}

PHullShader D3D11Video::createHullShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "hs_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11HullShader *pHullShader = nullptr;
	pDevice->CreateHullShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pHullShader);
	return std::make_shared<D3D11HullShader>(pHullShader);
}

PDomainShader D3D11Video::createDomainShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "ds_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11DomainShader *pDomainShader = nullptr;
	pDevice->CreateDomainShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pDomainShader);
	return std::make_shared<D3D11DomainShader>(pDomainShader);
}

PComputeShader D3D11Video::createComputeShader(const char *program, const char *entrypoint)
{
	ID3DBlob *pBytecode = createShader(program, entrypoint, "cs_5_0");
	if (pBytecode == nullptr)
		return nullptr;

	ID3D11ComputeShader *pComputeShader = nullptr;
	pDevice->CreateComputeShader(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), nullptr, &pComputeShader);
	return std::make_shared<D3D11ComputeShader>(pComputeShader);
}

PMaterial D3D11Video::createMaterial()
{
	return std::make_shared<Material>(this);
}

PPass D3D11Video::createPass()
{
	return std::make_shared<D3D11Pass>(this);
}

void D3D11Video::setViewport(s32 width, s32 height)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	pContext->RSSetViewports(1, &viewport);
}

void D3D11Video::setTexture(PTexture _texture)
{
	D3D11Texture *texture = (D3D11Texture *)_texture.get();
	ID3D11ShaderResourceView *shaderResourceView = texture->getShaderResourceView();
	pContext->PSSetShaderResources(0, 1, &shaderResourceView);
}

void D3D11Video::setModelMatrix(const Matrix4 &matrix)
{
	modelMatrix = matrix;
}

void D3D11Video::setViewMatrix(const Matrix4 &matrix)
{
	viewMatrix = matrix;
}

void D3D11Video::setProjectionMatrix(const Matrix4 &matrix)
{
	projectionMatrix = matrix;
}

void D3D11Video::setVertexShader(PVertexShader _vertexShader)
{
	std::shared_ptr<D3D11VertexShader> vertexShader = std::static_pointer_cast<D3D11VertexShader>(_vertexShader);
	pContext->VSSetShader(vertexShader->getVertexShader(), nullptr, 0);
	currentVertexShader = std::weak_ptr<D3D11VertexShader>(vertexShader);
}

void D3D11Video::setPixelShader(PPixelShader pixelShader)
{
	pContext->PSSetShader(static_cast<D3D11PixelShader *>(pixelShader.get())->getPixelShader(), nullptr, 0);
}

void D3D11Video::setGeometryShader(PGeometryShader geometryShader)
{
	pContext->GSSetShader(static_cast<D3D11GeometryShader *>(geometryShader.get())->getGeometryShader(), nullptr, 0);
}

void D3D11Video::setHullShader(PHullShader hullShader)
{
	pContext->HSSetShader(static_cast<D3D11HullShader *>(hullShader.get())->getHullShader(), nullptr, 0);
}

void D3D11Video::setDomainShader(PDomainShader domainShader)
{
	pContext->DSSetShader(static_cast<D3D11DomainShader *>(domainShader.get())->getDomainShader(), nullptr, 0);
}

void D3D11Video::setComputeShader(PComputeShader computeShader)
{
	pContext->CSSetShader(static_cast<D3D11ComputeShader *>(computeShader.get())->getComputeShader(), nullptr, 0);
}

Vector2DI D3D11Video::getBackBufferSize() const
{
	return Vector2DI(width, height);
}

void D3D11Video::beginDraw(Color clearColor)
{
	vertexCount = 0;
	pContext->ClearRenderTargetView(pBackBufferRenderTargetView, (const FLOAT *) &Colorf(clearColor));
	pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11Video::endDraw()
{
	pSwapChain->Present(0, 0);
}

void D3D11Video::setMaterial(PMaterial material)
{
	/* TODO: Multi-pass */
	D3D11Pass *pass = static_cast<D3D11Pass *>(material->getPass(0).get());
	pass->apply();
}

void D3D11Video::draw(
	PVertexBuffer _vertexBuffer,
	u32 startVertex,
	u32 count,
	PrimitiveTopology topology)
{
	D3D11VertexBuffer *vertexBuffer = static_cast<D3D11VertexBuffer *>(_vertexBuffer.get());

	/* Set input layout */
	std::shared_ptr<D3D11VertexShader> vertexShader = currentVertexShader.lock();
	if (!vertexShader) /* TODO: Add trace */
		return;
	pContext->IASetInputLayout(((D3D11VertexFormat *)vertexBuffer->getVertexFormat().get())->getInputLayout(pDevice, vertexShader->getBytecode()));

	/* Set matrix */
	D3D11_MAPPED_SUBRESOURCE pResource;
	pContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pResource);
	*(Matrix4 *)pResource.pData = modelMatrix * viewMatrix * projectionMatrix;
	pContext->Unmap(pMatrixBuffer, 0);
	pContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

	/* Set buffers */
	UINT stride = vertexBuffer->getVertexFormat()->getSize();
	UINT offsets = 0;
	ID3D11Buffer *vb = vertexBuffer->getVertexBuffer();
	pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offsets);
	pContext->IASetPrimitiveTopology(D3D11VertexFormat::getTopologyMapping(topology));

	/* Draw */
	pContext->Draw(count, startVertex);

	vertexCount += count;
}

void D3D11Video::drawIndexed(
	PVertexBuffer _vertexBuffer,
	u32 startVertex,
	PIndexBuffer _indexBuffer,
	u32 startIndex,
	u32 count,
	PrimitiveTopology topology
	)
{
	D3D11VertexBuffer *vertexBuffer = static_cast<D3D11VertexBuffer *>(_vertexBuffer.get());
	D3D11IndexBuffer *indexBuffer = static_cast<D3D11IndexBuffer *>(_indexBuffer.get());

	/* Set input layout */
	std::shared_ptr<D3D11VertexShader> vertexShader = currentVertexShader.lock();
	if (!vertexShader) /* TODO: Add trace */
		return;
	pContext->IASetInputLayout(((D3D11VertexFormat *)vertexBuffer->getVertexFormat().get())->getInputLayout(pDevice, vertexShader->getBytecode()));

	/* Set matrix */
	D3D11_MAPPED_SUBRESOURCE pResource;
	pContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pResource);
	*(Matrix4 *)pResource.pData = modelMatrix * viewMatrix * projectionMatrix;
	pContext->Unmap(pMatrixBuffer, 0);
	pContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

	/* Set buffers */
	UINT stride = vertexBuffer->getVertexFormat()->getSize();
	UINT offsets = 0;
	ID3D11Buffer *vb = vertexBuffer->getVertexBuffer();
	pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offsets);
	pContext->IASetIndexBuffer(indexBuffer->getIndexBuffer(),
		D3D11VertexFormat::getFormatMapping(indexBuffer->getType()),
		0);
	pContext->IASetPrimitiveTopology(D3D11VertexFormat::getTopologyMapping(topology));

	/* Draw */
	pContext->DrawIndexed(count, startIndex, startVertex);

	vertexCount += count;
}
