#pragma once

class IVideo;
class IShader;
class Material final
{
public:
	Material(IVideo *video);
	Material(const Material &) = delete;

	Material &operator= (const Material &) = delete;
	void setShaders(const char *program, const char *vertexShaderEntrypoint, const char *pixelShaderEntrypoint);

	void apply(); /* TODO */

private:
	IVideo *video;
	PVertexShader vertexShader;
	PPixelShader pixelShader;
};
