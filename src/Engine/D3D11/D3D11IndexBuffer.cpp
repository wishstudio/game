#include <stdafx.h>

#include <d3d11.h>

#include "D3D11IndexBuffer.h"
#include "D3D11Video.h"

D3D11IndexBuffer::D3D11IndexBuffer(D3D11Video *_video, VertexElementType _type, u32 _size):
	video(_video), type(_type), size(_size)
{
	ID3D11Device *device = video->getDevice();
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.ByteWidth = getTypeSize(type) * size;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&indexBufferDesc, nullptr, &pIndexBuffer);
}

D3D11IndexBuffer::~D3D11IndexBuffer()
{
	if (pIndexBuffer)
		pIndexBuffer->Release();
}

void D3D11IndexBuffer::update(u32 startIndex, u32 count, const void *data)
{
	ID3D11DeviceContext *context = video->getDeviceContext();
	D3D11_BOX box;
	box.left = startIndex * getTypeSize(type);
	box.right = (startIndex + count) * getTypeSize(type);
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	context->UpdateSubresource(pIndexBuffer, 0, &box, data, 0, 0);
}
