#include <Core.h>

#include "Material.h"
#include "Shader.h"
#include "Video.h"

Material::Material(Video *_video):
	video(_video)
{
}

PPass Material::createPass()
{
	PPass pass = video->createPass();
	passes.push_back(pass);
	return pass;
}
