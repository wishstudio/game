#pragma once

#include "../IIndexBuffer.h"

class D3D11Video;
class ID3D11Buffer;
class D3D11IndexBuffer : public IIndexBuffer
{
public:
	D3D11IndexBuffer(D3D11Video *video, VertexElementType type, u32 size);
	D3D11IndexBuffer(const D3D11IndexBuffer &) = delete;
	virtual ~D3D11IndexBuffer();

	D3D11IndexBuffer &operator= (const D3D11IndexBuffer &) = delete;

	virtual VertexElementType getType() const override { return type; }
	virtual u32 getSize() const override { return size; }
	virtual void update(u32 startIndex, u32 index, const void *data) override;

	/* INTERNAL INTERFACE */
	ID3D11Buffer *getIndexBuffer() const { return pIndexBuffer; }

private:
	D3D11Video *video;
	VertexElementType type;
	u32 size;
	ID3D11Buffer *pIndexBuffer = nullptr;
};
