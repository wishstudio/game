#pragma once

class Material
{
public:
	Material(PVideo video);
	Material(const Material &) = delete;

	Material &operator= (const Material &) = delete;
	void setShaders(const char *program, const char *vertexShaderEntrypoint, const char *pixelShaderEntrypoint);

	void apply(); /* TODO */

private:
	PVideo video;
	PVertexShader vertexShader;
	PPixelShader pixelShader;
};
