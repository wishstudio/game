#pragma once

#include "KeyValue.h"

class IWindowSystem
{
public:
	IWindowSystem();
	virtual ~IWindowSystem() {}

	virtual bool init(int width, int height) = 0;
	virtual bool setWindowSize(int width, int height) = 0;
	virtual void getWindowSize(int *width, int *height) = 0;
	virtual void setWindowTitle(const wchar_t *title) = 0;
	virtual void showError(const wchar_t *error) = 0;
	virtual void showError(const char *error) = 0;
	virtual bool processMessage() = 0;

	bool isKeyDown(KeyValue key) const { return keyDownState[key]; }
	bool isKeyPressed(KeyValue key) const { return keyPressState[key]; }

	void onKeyDown(KeyValue key);
	void onKeyUp(KeyValue key);
	void onNewFrame();
	void clearKeyState();

private:
	bool keyDownState[256], keyPressState[256];
};
