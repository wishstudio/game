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

void IWindowSystem::onNewFrame()
{
	memset(keyPressState, 0, sizeof keyPressState);
}

void IWindowSystem::onLostFocus()
{
	memset(keyPressState, 0, sizeof keyPressState);
	memset(keyDownState, 0, sizeof keyDownState);
}

void IWindowSystem::onMouseMove(int x, int y)
{
	int width, height;
	getWindowSize(&width, &height);
	f32 nx = 2.f * (f32)x / (f32)width - 1.f;
	f32 ny = 2.f * (f32)(height - y) / (f32)height - 1.f;
	normalizedMousePosition = Vector2D(nx, ny);
}

void IWindowSystem::setNormalizedMousePosition(f32 nx, f32 ny)
{
	int width, height;
	getWindowSize(&width, &height);
	int x = (nx + 1.f) / 2.f * (f32)width;
	int y = height - (ny + 1.f) / 2.f * (f32)height;
	setMousePosition(x, y);
}