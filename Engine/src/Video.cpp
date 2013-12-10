#include <Core.h>

#include "Video.h"
#include "D3D11/D3D11Video.h"
#include "Device/Win32Device.h"

PVideo Video::createVideo(u32 width, u32 height)
{
	std::shared_ptr<Win32Device> device = std::make_shared<Win32Device>();
	device->init(width, height);

	D3D11Video *video = new D3D11Video();
	video->init(device);
	return PVideo(video);
}
