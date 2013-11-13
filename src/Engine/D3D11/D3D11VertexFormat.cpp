#include <stdafx.h>

#include <d3d11.h>
#include "D3D11VertexFormat.h"

/* Unsupported combination is marked as DXGI_FORMAT_UNKNOWN */
static const DXGI_FORMAT ELEMENT_TYPE_MAPPING[TYPE_COUNT] = {
	DXGI_FORMAT_R8_SINT,   DXGI_FORMAT_R8G8_SINT,    DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R8G8B8A8_SINT,      /* BYTE */
	DXGI_FORMAT_R8_SNORM,  DXGI_FORMAT_R8G8_SNORM,   DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R8G8B8A8_SNORM,     /* BYTE_NORM */
	DXGI_FORMAT_R8_UINT,   DXGI_FORMAT_R8G8_UINT,    DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R8G8B8A8_UINT,      /* UBYTE */
	DXGI_FORMAT_R8_UNORM,  DXGI_FORMAT_R8G8_UNORM,   DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R8G8B8A8_UNORM,     /* UBYTE_NORM */
	DXGI_FORMAT_R16_SINT,  DXGI_FORMAT_R16G16_SINT,  DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R16G16B16A16_SINT,  /* SHORT */
	DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R16G16B16A16_SNORM, /* SHORT_NORM */
	DXGI_FORMAT_R16_UINT,  DXGI_FORMAT_R16G16_UINT,  DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R16G16B16A16_UINT,  /* USHORT */
	DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_R16G16B16A16_UNORM, /* USHORT_NORM */
	DXGI_FORMAT_R32_SINT,  DXGI_FORMAT_R32G32_SINT,  DXGI_FORMAT_R32G32B32_SINT,  DXGI_FORMAT_R32G32B32A32_SINT,  /* INT */
	DXGI_FORMAT_R32_UINT,  DXGI_FORMAT_R32G32_UINT,  DXGI_FORMAT_R32G32B32_UINT,  DXGI_FORMAT_R32G32B32A32_UINT,  /* UINT */
	DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, /* FLOAT */
	DXGI_FORMAT_UNKNOWN,   DXGI_FORMAT_UNKNOWN,      DXGI_FORMAT_UNKNOWN,         DXGI_FORMAT_UNKNOWN,            /* DOUBLE */
};

static const D3D11_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY_MAPPING[TOPOLOGY_COUNT] = {
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
};

D3D11VertexFormat::D3D11VertexFormat()
{
}

D3D11VertexFormat::~D3D11VertexFormat()
{
	clearInputLayoutCache();
}

void D3D11VertexFormat::addElement(VertexElementType type, VertexElementSemantic semantic, u32 semanticIndex)
{
	D3D11_INPUT_ELEMENT_DESC desc;
	desc.SemanticName = getSemanticName(semantic);
	desc.SemanticIndex = semanticIndex;
	desc.Format = getFormatMapping(type);
	desc.InputSlot = 0;
	desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc.InstanceDataStepRate = 0;
	elements.push_back(std::move(desc));
	dirty = true;
	size += getTypeSize(type);
}

ID3D11InputLayout *D3D11VertexFormat::getInputLayout(ID3D11Device *device, ID3DBlob *shaderData)
{
	if (dirty)
		clearInputLayoutCache();
	auto it = shaderInputLayouts.find(shaderData);
	if (it != shaderInputLayouts.end())
		return it->second;

	ID3D11InputLayout *pInputLayout;
	device->CreateInputLayout(elements.data(), elements.size(), shaderData->GetBufferPointer(), shaderData->GetBufferSize(), &pInputLayout);
	shaderInputLayouts.insert(std::make_pair(shaderData, pInputLayout));
	return pInputLayout;
}

DXGI_FORMAT D3D11VertexFormat::getFormatMapping(VertexElementType type)
{
	return ELEMENT_TYPE_MAPPING[type]; /* TODO: Throw exception on unsupported formats */
}

D3D11_PRIMITIVE_TOPOLOGY D3D11VertexFormat::getTopologyMapping(PrimitiveTopology topology)
{
	return PRIMITIVE_TOPOLOGY_MAPPING[topology]; /* TODO: Throw exception on unsupported topologies */
}

void D3D11VertexFormat::clearInputLayoutCache()
{
	for (auto it : shaderInputLayouts)
		it.second->Release();
	shaderInputLayouts.clear();
}

const char *D3D11VertexFormat::getSemanticName(VertexElementSemantic semantic) const
{
	switch (semantic)
	{
	case SEMANTIC_POSITION:
		return "SV_POSITION";

	case SEMANTIC_NORMAL:
		return "NORMAL";

	case SEMANTIC_COLOR:
		return "COLOR";

	case SEMANTIC_TEXCOORD:
		return "TEXCOORD";
	}
	/* Make compiler happy */
	/* TODO : Maybe throw here? */
	return "";
}
