#pragma once

class Thread
{
public:
	Thread();
	virtual ~Thread();

	void start();
	void stop();
	void suspend();
	void resume();

protected:
	virtual void run() = 0;
	bool shouldStop() const { return stopFlag; };

private:
	volatile bool stopFlag;
	HANDLE threadHandle;
};
