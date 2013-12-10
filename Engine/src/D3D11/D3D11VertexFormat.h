#pragma once

#include <unordered_map>
#include <vector>

#include <d3d11.h>

#include "../VertexFormat.h"

class D3D11VertexFormat final: public VertexFormat
{
public:
	D3D11VertexFormat();
	D3D11VertexFormat(const D3D11VertexFormat &) = delete;
	virtual ~D3D11VertexFormat();

	D3D11VertexFormat &operator= (const D3D11VertexFormat &) = delete;

	virtual u32 getSize() const override { return size; }
	virtual void addElement(VertexElementType type, VertexElementSemantic semantic, u32 semanticIndex = 0) override;

	/* INTERNAL INTERFACE */
	ID3D11InputLayout *getInputLayout(ID3D11Device *device, ID3DBlob *shaderData);
	static DXGI_FORMAT getFormatMapping(VertexElementType type);
	static D3D11_PRIMITIVE_TOPOLOGY getTopologyMapping(PrimitiveTopology topology);

private:
	void clearInputLayoutCache();
	const char *getSemanticName(VertexElementSemantic semantic) const;

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	std::unordered_map<ID3DBlob *, ID3D11InputLayout *> shaderInputLayouts;
	bool dirty = false;
	u32 size = 0;
};
