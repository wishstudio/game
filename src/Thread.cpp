#include "stdafx.h"

#include "Thread.h"

struct CallInfo
{
	Thread *obj;
	void (Thread::*func)();
};

static DWORD WINAPI ThreadProc(LPVOID parameter)
{
	CallInfo *info = (CallInfo *) parameter;
	(info->obj->*(info->func))();
	delete info;
	return 0;
}

Thread::Thread()
{
	stopFlag = false;
}

Thread::~Thread()
{
}

void Thread::start()
{
	CallInfo *info = new CallInfo();
	info->obj = this;
	info->func = &Thread::run;
	threadHandle = CreateThread(nullptr, 0, ThreadProc, info, 0, nullptr);
}

void Thread::stop()
{
	stopFlag = true;
	resume();
	WaitForSingleObject(threadHandle, INFINITE);
}

void Thread::suspend()
{
	SuspendThread(threadHandle);
}

void Thread::resume()
{
	ResumeThread(threadHandle);
}
