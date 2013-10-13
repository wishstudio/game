#include "stdafx.h"

#include "EventReceiver.h"

EventReceiver::EventReceiver()
	: IEventReceiver()
{
	leftButtonDown = false;
	rightButtonDown = false;
	mouseWheel = 0.f;
	for (int i = 0; i < KEY_KEY_CODES_COUNT; ++i)
		keyIsDown[i] = false;
}

EventReceiver::~EventReceiver()
{
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
		keyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
	return false;
}
