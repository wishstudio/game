#include "stdafx.h"

#include "EventReceiver.h"

EventReceiver::EventReceiver()
	: IEventReceiver()
{
	leftButtonDown = false;
	rightButtonDown = false;
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
		}
	}
	return false;
}
