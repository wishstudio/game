#include <stdafx.h>

#include "IVideo.h"
#include "Material.h"
#include "Shader.h"

Material::Material(IVideo *_video):
	video(_video)
{
}

void Material::setShaders(const char *program, const char *vertexShaderEntrypoint, const char *pixelShaderEntrypoint)
{
	vertexShader = video->createVertexShader(program, vertexShaderEntrypoint);
	pixelShader = video->createPixelShader(program, pixelShaderEntrypoint);
}

void Material::apply()
{
	video->setVertexShader(vertexShader);
	video->setPixelShader(pixelShader);
}
