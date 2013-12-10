#pragma once

#include "../VertexBuffer.h"

class D3D11Video;
struct ID3D11Buffer;
class D3D11VertexBuffer: public VertexBuffer
{
public:
	D3D11VertexBuffer(D3D11Video *video, PVertexFormat format, u32 size);
	D3D11VertexBuffer(const D3D11VertexBuffer &) = delete;
	virtual ~D3D11VertexBuffer();

	D3D11VertexBuffer &operator= (const D3D11VertexBuffer &) = delete;

	virtual PVertexFormat getVertexFormat() override { return format; }
	virtual u32 getSize() const override { return size; }
	virtual void update(u32 startIndex, u32 index, const void *data) override;

	/* INTERNAL INTERFACE */
	ID3D11Buffer *getVertexBuffer() const { return pVertexBuffer; }

private:
	D3D11Video *video;
	PVertexFormat format;
	u32 size;
	ID3D11Buffer *pVertexBuffer = nullptr;
};
