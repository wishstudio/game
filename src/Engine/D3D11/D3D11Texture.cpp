#include <stdafx.h>

#include <d3d11.h>

#include "D3D11Texture.h"

D3D11Texture::~D3D11Texture()
{
	if (pTexture2D)
		pTexture2D->Release();
	if (pShaderResourceView)
		pShaderResourceView->Release();
}
