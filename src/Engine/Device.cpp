#include <stdafx.h>

#include <cstring>
#include "Device.h"

Device::Device()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
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
	f32 nx = 2.f * (f32)x / (f32)width - 1.f;
	f32 ny = 2.f * (f32)(height - y) / (f32)height - 1.f;
	normalizedMousePosition = Vector2D(nx, ny);
}

void Device::onLostFocus()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mouseDownState, 0, sizeof mouseDownState);
	memset(mousePressState, 0, sizeof mousePressState);
}

void Device::onNewFrame(u64 _currentTimeMicroseconds)
{
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mousePressState, 0, sizeof mousePressState);
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
			averageFrameTimeMicroseconds = (currentTimeMicroseconds - fpsLastTimeMicroseconds) / fpsFrameCount;
			fpsLastTimeMicroseconds = currentTimeMicroseconds;
			fpsFrameCount = 0;
		}
	}
}

void Device::setNormalizedMousePosition(f32 nx, f32 ny)
{
	u32 width, height;
	getWindowSize(&width, &height);
	int x = (nx + 1.f) / 2.f * (f32)width;
	int y = height - (ny + 1.f) / 2.f * (f32)height;
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
