#include <stdafx.h>

#include <d3dcompiler.h>
#include "../WindowSystem/Win32WindowSystem.h"
#include "D3D11Video.h"

struct Texture
{
	ID3D11Texture2D *pTexture;
	ID3D11ShaderResourceView *pResourceView;
	int width, height;
};

typedef HRESULT(_stdcall * PWICCBS)(REFWICPixelFormatGUID, IWICBitmapSource *, IWICBitmapSource **ppIDst);
static PWICCBS _WICConvertBitmapSource;

static const char SHADER_SRC[] = R"DELIM(
cbuffer MatrixBuffer
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};

struct VS_InputType
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

struct PS_InputType
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

PS_InputType VS_Main(VS_InputType input)
{
	PS_InputType output = input;
	output.pos = mul(output.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);
	return output;
}

Texture2D shaderTexture: register(t0);
SamplerState shaderSampler: register(s0);

float4 PS_Main(PS_InputType input): SV_TARGET
{
	return shaderTexture.Sample(shaderSampler, input.tex) * input.color;
}
)DELIM";

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

	if (pInputLayout)
		pInputLayout->Release();
	if (pVertexShader)
		pVertexShader->Release();
	if (pPixelShader)
		pPixelShader->Release();

	if (pMatrixBuffer)
		pMatrixBuffer->Release();

	if (pSamplerState)
		pSamplerState->Release();
}

bool D3D11Video::init(Win32WindowSystem *windowSystem)
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
	windowSystem->getWindowSize(&width, &height);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof swapChainDesc);
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = windowSystem->getWindowHandle();
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

	createShaders();

	/* Matrix buffer */
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(MatrixBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	hr = pDevice->CreateBuffer(&desc, NULL, &pMatrixBuffer);
	if (FAILED(hr))
		return false;

	/* Sampler */
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof samplerDesc);
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	if (FAILED(hr))
		return false;
	pContext->PSSetSamplers(0, 1, &pSamplerState);

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
	rasterizerStateDesc.AntialiasedLineEnable = false;
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

void D3D11Video::clearScreen()
{
	//float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearColor[4] = { 127.0 / 255.0f, 200 / 255.0f, 251 / 255.0f, 255.0 / 255.0f };
	pContext->ClearRenderTargetView(pBackBufferRenderTargetView, clearColor);
	pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11Video::createShaders()
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
	PD3DCOMPILE D3DCompile = (PD3DCOMPILE) GetProcAddress(hm, "D3DCompile");

	const u32 size = sizeof(SHADER_SRC);
	ID3DBlob *errMsg;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;

	/* Vertex shader */
	ID3DBlob *vsBlob;
	D3DCompile(SHADER_SRC, size, nullptr, nullptr, nullptr, "VS_Main", "vs_5_0", compileFlags, 0, &vsBlob, &errMsg);
	if (errMsg)
	{
		printf("Vertex shader compilation message:\n%s\n", errMsg->GetBufferPointer());
		errMsg->Release();
	}
	pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pVertexShader);

	/* Pixel shader */
	ID3DBlob *psBlob;
	D3DCompile(SHADER_SRC, size, nullptr, nullptr, nullptr, "PS_Main", "ps_5_0", compileFlags, 0, &psBlob, &errMsg);
	if (errMsg)
	{
		printf("Pixel shader compilation message:\n%s\n", errMsg->GetBufferPointer());
		errMsg->Release();
	}
	pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pPixelShader);

	/* Input layout */
	D3D11_INPUT_ELEMENT_DESC vertexShader2DInputDesc[] =
	{
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	pDevice->CreateInputLayout(vertexShader2DInputDesc, ARRAYSIZE(vertexShader2DInputDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &pInputLayout);

	vsBlob->Release();
	psBlob->Release();
}

void D3D11Video::beginDraw()
{
}

void D3D11Video::endDraw()
{
	pSwapChain->Present(0, 0);
}

char *D3D11Video::decodeImage(const char *raw, int size, int *width, int *height)
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

char *D3D11Video::getResourceData(const char *resourceName, int *fileSize)
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
	(*fileSize) = (int)size.QuadPart;
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

ITexture *D3D11Video::createTexture(int width, int height, const void *initialData, D3D11_USAGE usage, UINT bindFlag)
{
	Texture *texture = new Texture();
	memset(texture, 0, sizeof texture);
	texture->width = width;
	texture->height = height;
	HRESULT hr;

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
		pDevice->CreateTexture2D(&textureDesc, &dataDesc, &texture->pTexture);
	}
	else
		pDevice->CreateTexture2D(&textureDesc, NULL, &texture->pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	resourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = 1;

	pDevice->CreateShaderResourceView(texture->pTexture, &resourceViewDesc, &texture->pResourceView);
	return (ITexture *) texture;
};

ITexture *D3D11Video::createTexture(const std::string &path)
{
	int size;
	char *raw = getResourceData(path.c_str(), &size);
	if (raw == nullptr)
		return nullptr;
	int width, height;
	char *image = decodeImage(raw, size, &width, &height);
	delete raw;
	if (image == NULL)
		return NULL;
	ITexture *texture = createTexture(width, height, image, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE);
	delete image;
	return texture;
}

void D3D11Video::deleteTexture(ITexture *_texture)
{
	Texture *texture = (Texture *) _texture;
	if (texture->pResourceView)
		texture->pResourceView->Release();
	if (texture->pTexture)
		texture->pTexture->Release();
	delete texture;
}

MeshBuffer *D3D11Video::createMeshBuffer()
{
	return new MeshBuffer();
}

void D3D11Video::deleteMeshBuffer(MeshBuffer *buffer)
{
	if (buffer->pVertexBuffer)
		buffer->pVertexBuffer->Release();
	if (buffer->pIndexBuffer)
		buffer->pIndexBuffer->Release();
	delete buffer;
}

void D3D11Video::setTexture(ITexture *_texture)
{
	Texture *texture = (Texture *)_texture;
	pContext->PSSetShaderResources(0, 1, &texture->pResourceView);
}

void D3D11Video::setModelMatrix(const Matrix4 &matrix)
{
	matrixBuffer.model = matrix;
}

void D3D11Video::setViewMatrix(const Matrix4 &matrix)
{
	matrixBuffer.view = matrix;
}

void D3D11Video::setProjectionMatrix(const Matrix4 &matrix)
{
	matrixBuffer.projection = matrix;
}

void D3D11Video::updateHardwareBuffer(MeshBuffer *buffer)
{
	if (buffer->vertexDirty)
	{
		if (buffer->pVertexBuffer)
			buffer->pVertexBuffer->Release();

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.ByteWidth = sizeof(Vertex) * buffer->vertexBuffer.size();
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA resourceData;
		resourceData.pSysMem = buffer->vertexBuffer.data();
		resourceData.SysMemPitch = 0;
		resourceData.SysMemSlicePitch = 0;
		pDevice->CreateBuffer(&bufferDesc, &resourceData, &buffer->pVertexBuffer);

		buffer->vertexDirty = false;
	}
	if (buffer->indexDirty)
	{
		if (buffer->pIndexBuffer)
			buffer->pIndexBuffer->Release();

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.ByteWidth = sizeof(u32) * buffer->indexBuffer.size();
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA resourceData;
		resourceData.pSysMem = buffer->indexBuffer.data();
		resourceData.SysMemPitch = 0;
		resourceData.SysMemSlicePitch = 0;
		pDevice->CreateBuffer(&bufferDesc, &resourceData, &buffer->pIndexBuffer);

		buffer->indexDirty = false;
	}
}

void D3D11Video::drawMeshBuffer(MeshBuffer *buffer)
{
	updateHardwareBuffer(buffer);

	/* Set matrix */
	D3D11_MAPPED_SUBRESOURCE pResource;
	pContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pResource);
	MatrixBuffer *matrix = (MatrixBuffer *) pResource.pData;
	matrix->model = matrixBuffer.model;
	matrix->view = matrixBuffer.view;
	matrix->projection = matrixBuffer.projection;
	pContext->Unmap(pMatrixBuffer, 0);
	pContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

	/* Set shaders */
	pContext->VSSetShader(pVertexShader, nullptr, 0);
	pContext->PSSetShader(pPixelShader, nullptr, 0);

	/* Set input layout */
	pContext->IASetInputLayout(pInputLayout);

	/* Set buffers */
	UINT stride = sizeof(Vertex);
	UINT offsets = 0;
	pContext->IASetVertexBuffers(0, 1, &buffer->pVertexBuffer, &stride, &offsets);
	pContext->IASetIndexBuffer(buffer->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/* Draw buffer */
	pContext->DrawIndexed(buffer->indexBuffer.size(), 0, 0);
}
