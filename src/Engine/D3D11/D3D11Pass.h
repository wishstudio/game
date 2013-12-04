#pragma once

#include "../Pass.h"

class ID3D11RasterizerState;
class D3D11Video;
class D3D11Pass final: public Pass
{
public:
	D3D11Pass(D3D11Video *video);
	virtual ~D3D11Pass();

	/* INTERNAL INTERFACE */
	void apply();

private:
	D3D11Video *video;
	ID3D11RasterizerState *rasterizerState = nullptr;
};
