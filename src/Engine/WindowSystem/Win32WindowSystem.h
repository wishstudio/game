#ifndef _WIN32WINDOWSYSTEM_H
#define _WIN32WINDOWSYSTEM_H

#include <Windows.h>
#include "../IWindowSystem.h"

class Win32WindowSystem: public IWindowSystem
{
public:
	Win32WindowSystem();
	virtual ~Win32WindowSystem();

	/* Win32 specific window system functions */
	LPWSTR getWindowClassName() const { return L"dxlearn"; }
	HINSTANCE getInstance() const { return hInstance; }
	HWND getWindowHandle() const { return windowHandle; }
	
	/* Generic window system functions */
	virtual bool init(int width, int height) override;
	virtual bool setWindowSize(int width, int height) override;
	virtual void getWindowSize(int *width, int *height) override;
	virtual void setWindowTitle(const wchar_t *title) override;
	virtual void showError(const wchar_t *error) override;
	virtual void showError(const char *error) override;
	virtual bool processMessage() override;

private:
	HINSTANCE hInstance;
	HWND windowHandle;
	int w, h;
};

#endif