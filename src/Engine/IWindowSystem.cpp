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

void IWindowSystem::clearKeyState()
{
	memset(keyPressState, 0, sizeof keyPressState);
	memset(keyDownState, 0, sizeof keyDownState);
}
