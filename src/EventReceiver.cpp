#include "stdafx.h"

#include "EventReceiver.h"

EventReceiver::EventReceiver()
	: IEventReceiver()
{
	leftButtonDown = false;
	rightButtonDown = false;
	mouseWheel = 0.f;
	memset(keyIsDown, 0, sizeof(keyIsDown));
	memset(keyIsPressed, 0, sizeof(keyIsPressed));
}


EventReceiver::~EventReceiver()
{
}


f32 EventReceiver::getMouseWheel() {
	f32 wheel = mouseWheel;
	mouseWheel = 0;
	return wheel;
}

bool EventReceiver::OnEvent(const SEvent &event)
{
	if (event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		switch (event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
			leftButtonDown = true;
			break;

		case EMIE_LMOUSE_LEFT_UP:
			leftButtonDown = false;
			break;

		case EMIE_RMOUSE_PRESSED_DOWN:
			rightButtonDown = true;
			break;

		case EMIE_RMOUSE_LEFT_UP:
			rightButtonDown = false;
			break;
		case EMIE_MOUSE_WHEEL:
			mouseWheel += event.MouseInput.Wheel;
			break;
		}
	}
	else if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		keyIsPressed[event.KeyInput.Key] = (event.KeyInput.PressedDown && !keyIsDown[event.KeyInput.Key]);
		keyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
	}
	return false;
}

void EventReceiver::update()
{
	memset(keyIsPressed, 0, sizeof keyIsPressed);
}
