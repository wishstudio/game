#pragma once

class Pass
{
public:
	virtual ~Pass() {}

	Pass(const Pass &) = delete;
	Pass &operator= (const Pass &) = delete;

	/* Shaders */
	PVertexShader getVertexShader() const { return vertexShader; }
	void setVertexShader(PVertexShader vertexShader) { this->vertexShader = vertexShader; }
	PPixelShader getPixelShader() const { return pixelShader; }
	void setPixelShader(PPixelShader pixelShader) { this->pixelShader = pixelShader; }

	/* Rasterizer states */
	FillMode getFillMode() const { return fillMode; }
	void setFillMode(FillMode fillMode) { this->fillMode = fillMode; rasterizerDirty = true; }
	CullMode getCullMode() const { return cullMode; }
	void setCullMode(CullMode cullMode) { this->cullMode = cullMode; rasterizerDirty = true; }
	FrontMode getFrontMode() const { return frontMode; }
	void setFrontMode(FrontMode frontMode) { this->frontMode = frontMode; rasterizerDirty = true; }
	f32 getDepthBias() const { return depthBias; }
	void setDepthBias(f32 depthBias) { this->depthBias = depthBias; rasterizerDirty = true; }
	f32 getDepthBiasClamp() const { return depthBiasClamp; }
	void setDepthBiasClamp(f32 depthBiasClamp) { this->depthBiasClamp = depthBiasClamp; rasterizerDirty = true; }
	f32 getSlopeScaledDepthBias() const { return slopeScaledDepthBias; }
	void setSlopeScaledDepthBias(f32 slopeScaledDepthBias) { this->slopeScaledDepthBias = slopeScaledDepthBias; rasterizerDirty = true; }
	bool getDepthClipEnabled() const { return depthClipEnabled; }
	void setDepthClipEnabled(bool depthClipEnabled) { this->depthClipEnabled = depthClipEnabled; rasterizerDirty = true; }
	bool getScissorEnabled() const { return scissorEnabled; }
	void setScissorEnabled(bool scissorEnabled) { this->scissorEnabled = scissorEnabled; rasterizerDirty = true; }
	bool getMultisampleEnabled() const { return multisampleEnabled; }
	void setMultisampleEnabled(bool multisampleEnabled) { this->multisampleEnabled = multisampleEnabled; rasterizerDirty = true; }
	bool getAntialiasedLineEnabled() const { return antialiasedLineEnabled; }
	void setAntialiasedLineEnabled(bool antialiasedLineEnabled) { this->antialiasedLineEnabled = antialiasedLineEnabled; rasterizerDirty = true; }

protected:
	Pass() {}

	bool isRasterizerDirty() const { return rasterizerDirty; }
	void clearDirtyFlag()
	{
		rasterizerDirty = false;
	}

private:
	/* Shaders */
	PVertexShader vertexShader;
	PPixelShader pixelShader;

	/* Rasterizer states */
	bool rasterizerDirty = true;
	FillMode fillMode = FILLMODE_SOLID;
	CullMode cullMode = CULLMODE_BACK;
	FrontMode frontMode = FRONTMODE_COUNTERCLOCKWISE;
	f32 depthBias = 0.f, depthBiasClamp = 0.f, slopeScaledDepthBias = 0.f;
	bool depthClipEnabled = true, scissorEnabled = false, multisampleEnabled = false, antialiasedLineEnabled = false;
};
