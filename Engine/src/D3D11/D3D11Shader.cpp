#include <Core.h>

#include <d3d11.h>

#include "D3D11Shader.h"

D3D11VertexShader::~D3D11VertexShader()
{
	if (pVertexShader)
		pVertexShader->Release();
	if (pBytecode)
		pBytecode->Release();
}

D3D11PixelShader::~D3D11PixelShader()
{
	if (pPixelShader)
		pPixelShader->Release();
}

D3D11GeometryShader::~D3D11GeometryShader()
{
	if (pGeometryShader)
		pGeometryShader->Release();
}

D3D11HullShader::~D3D11HullShader()
{
	if (pHullShader)
		pHullShader->Release();
}

D3D11DomainShader::~D3D11DomainShader()
{
	if (pDomainShader)
		pDomainShader->Release();
}

D3D11ComputeShader::~D3D11ComputeShader()
{
	if (pComputeShader)
		pComputeShader->Release();
}
