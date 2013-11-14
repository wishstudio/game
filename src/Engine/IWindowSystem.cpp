#include <stdafx.h>

#include <cstring>
#include "IWindowSystem.h"

IWindowSystem::IWindowSystem()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
}

void IWindowSystem::onKeyDown(KeyValue key)
{
	keyDownState[key] = true;
	keyPressState[key] = true;
}

void IWindowSystem::onKeyUp(KeyValue key)
{
	keyDownState[key] = false;
	keyPressState[key] = false;
}

void IWindowSystem::onMouseDown(MouseButton button)
{
	mouseDownState[button] = true;
	mousePressState[button] = true;
}

void IWindowSystem::onMouseUp(MouseButton button)
{
	mouseDownState[button] = false;
	mousePressState[button] = false;
}

void IWindowSystem::onMouseMove(int x, int y)
{
	int width, height;
	getWindowSize(&width, &height);
	f32 nx = 2.f * (f32)x / (f32)width - 1.f;
	f32 ny = 2.f * (f32)(height - y) / (f32)height - 1.f;
	normalizedMousePosition = Vector2D(nx, ny);
}

void IWindowSystem::onLostFocus()
{
	memset(keyDownState, 0, sizeof keyDownState);
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mouseDownState, 0, sizeof mouseDownState);
	memset(mousePressState, 0, sizeof mousePressState);
}

void IWindowSystem::onNewFrame(u64 _currentTimeMilliseconds)
{
	memset(keyPressState, 0, sizeof keyPressState);
	memset(mousePressState, 0, sizeof mousePressState);
	if (currentTickTimeMilliseconds == -1)
	{
		/* We are just initializing */
		initialTimeMilliseconds = _currentTimeMilliseconds;
		currentTickTimeMilliseconds = 0;
	}
	currentTimeMilliseconds = _currentTimeMilliseconds - initialTimeMilliseconds;
}

void IWindowSystem::setNormalizedMousePosition(f32 nx, f32 ny)
{
	int width, height;
	getWindowSize(&width, &height);
	int x = (nx + 1.f) / 2.f * (f32)width;
	int y = height - (ny + 1.f) / 2.f * (f32)height;
	setMousePosition(x, y);
}

bool IWindowSystem::tick()
{
	/* Calculate all things every time instead of accumulations to avoid cumulative error */
	/* (currentTick + 1) / ticksPerSecond <= currentTimeMilliseconds / 1000 */
	if ((currentTick + 1) * 1000 <= currentTimeMilliseconds * ticksPerSecond)
	{
		currentTick++;
		currentTickTimeMilliseconds = currentTick * 1000 / ticksPerSecond;
		return true;
	}
	return false;
}
