#pragma once

#include "../Texture.h"

class ID3D11Texture2D;
class ID3D11ShaderResourceView;
class D3D11Texture: public Texture
{
public:
	D3D11Texture(ID3D11Texture2D *_pTexture2D, ID3D11ShaderResourceView *_pShaderResourceView, u32 _width, u32 _height):
		pTexture2D(_pTexture2D), pShaderResourceView(_pShaderResourceView), width(_width), height(_height) {}
	virtual ~D3D11Texture();

	virtual u32 getWidth() const { return width; }
	virtual u32 getHeight() const { return height; }

	/* Internal interface */
	ID3D11Texture2D *getTexture2D() const { return pTexture2D; }
	ID3D11ShaderResourceView *getShaderResourceView() const { return pShaderResourceView; }

private:
	ID3D11Texture2D *pTexture2D;
	ID3D11ShaderResourceView *pShaderResourceView;
	u32 width, height;
};
