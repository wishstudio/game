#include <stdafx.h>

#include <d3dcompiler.h>
#include "../Device/Win32Device.h"
#include "D3D11IndexBuffer.h"
#include "D3D11Texture.h"
#include "D3D11VertexBuffer.h"
#include "D3D11VertexFormat.h"
#include "D3D11Video.h"

struct D3D11Shader
{
	union
	{
		IUnknown *pShader;
		ID3D11VertexShader *pVertexShader;
		ID3D11PixelShader *pPixelShader;
		ID3D11GeometryShader *pGeometryShader;
		ID3D11HullShader *pHullShader;
		ID3D11DomainShader *pDomainShader;
		ID3D11ComputeShader *pComputeShader;
	};
	ID3DBlob *pShaderBlob;
};

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

	int width, height;
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
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
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

IVertexFormat *D3D11Video::createVertexFormat()
{
	return new D3D11VertexFormat();
}

IVertexBuffer *D3D11Video::createVertexBuffer(IVertexFormat *format, u32 size)
{
	return new D3D11VertexBuffer(this, format, size);
}

IIndexBuffer *D3D11Video::createIndexBuffer(VertexElementType type, u32 size)
{
	return new D3D11IndexBuffer(this, type, size);
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

IVertexShader *D3D11Video::createVertexShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "vs_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pVertexShader);
	return (IVertexShader *) shader;
}

IPixelShader *D3D11Video::createPixelShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "ps_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pPixelShader);
	return (IPixelShader *) shader;
}

IGeometryShader *D3D11Video::createGeometryShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "gs_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pGeometryShader);
	return (IGeometryShader *) shader;
}

IHullShader *D3D11Video::createHullShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "hs_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pHullShader);
	return (IHullShader *) shader;
}

IDomainShader *D3D11Video::createDomainShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "ds_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pDomainShader);
	return (IDomainShader *) shader;
}

IComputeShader *D3D11Video::createComputeShader(const char *program, const char *entrypoint)
{
	ID3DBlob *blob = createShader(program, entrypoint, "cs_5_0");
	if (blob == nullptr)
		return nullptr;
	D3D11Shader *shader = new D3D11Shader();
	shader->pShaderBlob = blob;
	pDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader->pComputeShader);
	return (IComputeShader *) shader;
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

void D3D11Video::setVertexShader(IVertexShader *shader)
{
	pContext->VSSetShader(((D3D11Shader *)shader)->pVertexShader, nullptr, 0);
	currentVertexShader = (D3D11Shader *)shader;
}

void D3D11Video::setPixelShader(IPixelShader *shader)
{
	pContext->PSSetShader(((D3D11Shader *)shader)->pPixelShader, nullptr, 0);
}

void D3D11Video::setGeometryShader(IGeometryShader *shader)
{
	pContext->GSSetShader(((D3D11Shader *)shader)->pGeometryShader, nullptr, 0);
}

void D3D11Video::setHullShader(IHullShader *shader)
{
	pContext->HSSetShader(((D3D11Shader *)shader)->pHullShader, nullptr, 0);
}

void D3D11Video::setDomainShader(IDomainShader *shader)
{
	pContext->DSSetShader(((D3D11Shader *)shader)->pDomainShader, nullptr, 0);
}

void D3D11Video::setComputeShader(IComputeShader *shader)
{
	pContext->CSSetShader(((D3D11Shader *)shader)->pComputeShader, nullptr, 0);
}

void D3D11Video::beginDraw(Color clearColor)
{
	vertexCount = 0;
	//float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//float clearColor[4] = { 127.0 / 255.0f, 200 / 255.0f, 251 / 255.0f, 255.0 / 255.0f };
	pContext->ClearRenderTargetView(pBackBufferRenderTargetView, (const FLOAT *) &Colorf(clearColor));
	pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11Video::endDraw()
{
	pSwapChain->Present(0, 0);
}

void D3D11Video::draw(IVertexBuffer *vertexBuffer, u32 startVertex, u32 count, PrimitiveTopology topology)
{
	/* Set matrix */
	D3D11_MAPPED_SUBRESOURCE pResource;
	pContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pResource);
	*(Matrix4 *)pResource.pData = modelMatrix * viewMatrix * projectionMatrix;
	pContext->Unmap(pMatrixBuffer, 0);
	pContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

	/* Set input layout */
	pContext->IASetInputLayout(((D3D11VertexFormat *)vertexBuffer->getVertexFormat())->getInputLayout(pDevice, currentVertexShader->pShaderBlob));

	/* Set buffers */
	UINT stride = vertexBuffer->getVertexFormat()->getSize();
	UINT offsets = 0;
	ID3D11Buffer *vb = ((D3D11VertexBuffer *)vertexBuffer)->getVertexBuffer();
	pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offsets);
	pContext->IASetPrimitiveTopology(D3D11VertexFormat::getTopologyMapping(topology));

	/* Draw */
	pContext->Draw(count, startVertex);

	vertexCount += count;
}

void D3D11Video::drawIndexed(
	IVertexBuffer *vertexBuffer,
	u32 startVertex,
	IIndexBuffer *indexBuffer,
	u32 startIndex,
	u32 count,
	PrimitiveTopology topology
	)
{
	/* Set matrix */
	D3D11_MAPPED_SUBRESOURCE pResource;
	pContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pResource);
	*(Matrix4 *) pResource.pData = modelMatrix * viewMatrix * projectionMatrix;
	pContext->Unmap(pMatrixBuffer, 0);
	pContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

	/* Set input layout */
	pContext->IASetInputLayout(((D3D11VertexFormat *) vertexBuffer->getVertexFormat())->getInputLayout(pDevice, currentVertexShader->pShaderBlob));

	/* Set buffers */
	UINT stride = vertexBuffer->getVertexFormat()->getSize();
	UINT offsets = 0;
	ID3D11Buffer *vb = ((D3D11VertexBuffer *) vertexBuffer)->getVertexBuffer();
	pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offsets);
	pContext->IASetIndexBuffer(((D3D11IndexBuffer *) indexBuffer)->getIndexBuffer(),
		D3D11VertexFormat::getFormatMapping(indexBuffer->getType()),
		0);
	pContext->IASetPrimitiveTopology(D3D11VertexFormat::getTopologyMapping(topology));

	/* Draw */
	pContext->DrawIndexed(count, startIndex, startVertex);

	vertexCount += count;
}
