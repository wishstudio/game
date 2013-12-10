#include <stdafx.h>

#include <d3d11.h>

#include "D3D11VertexBuffer.h"
#include "D3D11Video.h"

D3D11VertexBuffer::D3D11VertexBuffer(D3D11Video *_video, PVertexFormat _format, u32 _size):
	video(_video), format(_format), size(_size)
{
	ID3D11Device *device = video->getD3D11Device();
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.ByteWidth = format->getSize() * size;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&vertexBufferDesc, nullptr, &pVertexBuffer);
}

D3D11VertexBuffer::~D3D11VertexBuffer()
{
	if (pVertexBuffer)
		pVertexBuffer->Release();
}

void D3D11VertexBuffer::update(u32 startIndex, u32 count, const void *data)
{
	ID3D11DeviceContext *context = video->getD3D11DeviceContext();
	D3D11_BOX box;
	box.left = startIndex * format->getSize();
	box.right = (startIndex + count) * format->getSize();
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	context->UpdateSubresource(pVertexBuffer, 0, &box, data, 0, 0);
}
