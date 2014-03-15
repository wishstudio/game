#include <Core.h>

#include <d3d11.h>

#include "../ShaderCore/D3D11/D3D11Backend.h"
#include "D3D11Shader.h"

D3D11Shader::D3D11Shader(ID3D11Device *device, D3D11Backend *_shaderReflection)
	: shaderReflection(_shaderReflection)
{
	for (int i = 0; i < shaderReflection->getConstantBufferCount(); i++)
	{
		D3D11ConstantBufferReflection *cbReflection = shaderReflection->getConstantBufferReflection(i);

		/* Create constant buffer */
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = cbReflection->getSize();
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		ID3D11Buffer *buffer;
		device->CreateBuffer(&desc, nullptr, &buffer);
		constantBuffers.push_back(buffer);

		/* Create system memory cache */
		ConstantBufferCache cache;
		cache.data.reset(new char[cbReflection->getSize()]);
		memset(cache.data.get(), 0, cbReflection->getSize());
		cache.dirty = true;
		constantBufferCaches.push_back(std::move(cache));

		/* Add constants to constants map */
		for (int i = 0; i < cbReflection->getConstantCount(); i++)
		{
			D3D11ConstantReflection *constantReflection = cbReflection->getConstantReflection(i);
			constantsMap.insert(std::make_pair(constantReflection->getName(), constantReflection));
		}
	}
}

void D3D11Shader::setShaderConstant(const std::string &name, const Matrix4 &matrix)
{
	D3D11ConstantReflection *constantReflection = constantsMap[name];
	//assert(constantReflection->getType()->getIsMatrix()); ...
	ConstantBufferCache &cache = constantBufferCaches[constantReflection->getConstantBufferSlot()];
	*(Matrix4 *) (cache.data.get() + constantReflection->getOffset()) = matrix;
	cache.dirty = true;
}

void D3D11Shader::syncConstantBuffers(ID3D11DeviceContext *context)
{
	for (u32 i = 0; i < constantBufferCaches.size(); i++)
	{
		ConstantBufferCache &cache = constantBufferCaches[i];
		if (cache.dirty)
		{
			D3D11_MAPPED_SUBRESOURCE pMappedResource;
			context->Map(constantBuffers[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &pMappedResource);
			memcpy(pMappedResource.pData, cache.data.get(), shaderReflection->getConstantBufferReflection(i)->getSize());
			context->Unmap(constantBuffers[i], 0);
			cache.dirty = false;
		}
	}
}

D3D11Shader::~D3D11Shader()
{
	for (ID3D11Buffer *buffer: constantBuffers)
		buffer->Release();
}

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
