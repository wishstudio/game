#include "stdafx.h"

#include "TimeManager.h"

void TimeManager::update()
{
	actualTime = clock.now();
	if (currentTime.time_since_epoch().count() == 0)
	{
		/* We are just beginning */
		currentTime = actualTime;
	}
}

bool TimeManager::tick()
{
	if (currentTime + TICK_DURATION <= actualTime)
	{
		currentTick++;
		currentTime += TICK_DURATION;
		return true;
	}
	return false;
}

std::chrono::high_resolution_clock::duration TimeManager::getRemainingTickDuration() const
{
	/* assert(currentTime + TICK_DURATION > actualTime); */
	return actualTime - currentTime;
}
