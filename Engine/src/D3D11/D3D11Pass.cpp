#include <Core.h>

#include <d3d11.h>

#include "D3D11Pass.h"
#include "D3D11Video.h"

static D3D11_FILL_MODE fillModeMap[] = {
	D3D11_FILL_WIREFRAME,
	D3D11_FILL_SOLID
};

static D3D11_CULL_MODE cullModeMap[] = {
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK
};

D3D11Pass::D3D11Pass(D3D11Video *_video):
	video(_video)
{
}

D3D11Pass::~D3D11Pass()
{
	if (rasterizerState)
		rasterizerState->Release();
}

void D3D11Pass::apply()
{
	ID3D11Device *device = video->getD3D11Device();
	ID3D11DeviceContext *context = video->getD3D11DeviceContext();
	if (isRasterizerDirty())
	{
		if (rasterizerState)
			rasterizerState->Release();

		D3D11_RASTERIZER_DESC desc;
		desc.FillMode = fillModeMap[getFillMode()];
		desc.CullMode = cullModeMap[getCullMode()];
		desc.FrontCounterClockwise = (getFrontMode() == FRONTMODE_COUNTERCLOCKWISE);
		desc.DepthBias = (INT)(getDepthBias() * (float)(1 << 23));
		desc.DepthBiasClamp = getDepthBiasClamp();
		desc.SlopeScaledDepthBias = getSlopeScaledDepthBias();
		desc.DepthClipEnable = getDepthClipEnabled();
		desc.ScissorEnable = getScissorEnabled();
		desc.MultisampleEnable = getMultisampleEnabled();
		desc.AntialiasedLineEnable = getAntialiasedLineEnabled();
		device->CreateRasterizerState(&desc, &rasterizerState);
	}

	video->setVertexShader(getVertexShader());
	video->setPixelShader(getPixelShader());
	context->RSSetState(rasterizerState);
	clearDirtyFlag();
}
