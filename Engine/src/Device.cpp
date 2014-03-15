#include <Core.h>

#include <cstring>
#include "Device.h"

Device::Device()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
	mouseWheel = 0;
}

void Device::onKeyDown(KeyValue key)
{
	keyDownState[key] = true;
	keyPressState[key] = true;
}

void Device::onKeyUp(KeyValue key)
{
	keyDownState[key] = false;
	keyPressState[key] = false;
}

void Device::onMouseDown(MouseButton button)
{
	mouseDownState[button] = true;
	mousePressState[button] = true;
}

void Device::onMouseUp(MouseButton button)
{
	mouseDownState[button] = false;
	mousePressState[button] = false;
}

void Device::onMouseMove(int x, int y)
{
	u32 width, height;
	getWindowSize(&width, &height);
	float nx = 2.f * (float)x / (float)width - 1.f;
	float ny = 2.f * (float)(height - y) / (float)height - 1.f;
	normalizedMousePosition = Vector2D(nx, ny);
}

void Device::onMouseWheel(s32 degree)
{
	mouseWheel = degree / 120.f;
}

void Device::onLostFocus()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mouseDownState, 0, sizeof mouseDownState);
	memset(mousePressState, 0, sizeof mousePressState);
	mouseWheel = 0;
}

void Device::onNewFrame(u64 _currentTimeMicroseconds)
{
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mousePressState, 0, sizeof mousePressState);
	mouseWheel = 0;
	if (initialTimeMicroseconds == -1)
	{
		/* We are just initializing */
		initialTimeMicroseconds = _currentTimeMicroseconds;
		currentTimeMicroseconds = 0;
		currentTickTimeMicroseconds = 0;
	}
	else
	{
		currentTimeMicroseconds = _currentTimeMicroseconds - initialTimeMicroseconds;
		fpsFrameCount++;
		if (currentTimeMicroseconds - fpsLastTimeMicroseconds >= 1000000)
		{
			averageFrameTimeMicroseconds = (u32)((currentTimeMicroseconds - fpsLastTimeMicroseconds) / fpsFrameCount);
			fpsLastTimeMicroseconds = (u32)currentTimeMicroseconds;
			fpsFrameCount = 0;
		}
	}
}

void Device::setNormalizedMousePosition(float nx, float ny)
{
	u32 width, height;
	getWindowSize(&width, &height);
	s32 x = (s32)((nx + 1.f) / 2.f * (float)width);
	s32 y = (s32)(height - (ny + 1.f) / 2.f * (float)height);
	setMousePosition(x, y);
}

bool Device::tick()
{
	/* Calculate all things every time instead of accumulations to avoid cumulative error */
	/* (currentTick + 1) / ticksPerSecond <= currentTimeMilliseconds / 1000000 */
	if ((currentTick + 1) * 1000000 <= currentTimeMicroseconds * ticksPerSecond)
	{
		currentTick++;
		currentTickTimeMicroseconds = currentTick * 1000000 / ticksPerSecond;
		return true;
	}
	return false;
}
