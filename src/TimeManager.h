#pragma once

class TimeManager final
{
public:
	void update();

	bool tick();
	u64 getTick() const { return currentTick; }
	std::chrono::high_resolution_clock::duration getRemainingTickDuration() const;

private:
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point currentTime, actualTime;
	u64 currentTick = 0;
};
