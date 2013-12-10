#pragma once

#include <Windows.h>
#include "../Device.h"

class Win32Device: public Device
{
public:
	Win32Device();
	virtual ~Win32Device();

	/* Win32 specific window system functions */
	LPWSTR getWindowClassName() const { return L"dxlearn"; }
	HINSTANCE getInstance() const { return hInstance; }
	HWND getWindowHandle() const { return windowHandle; }
	
	/* Generic window system functions */
	virtual bool init(u32 width, u32 height) override;
	virtual bool setWindowSize(u32 width, u32 height) override;
	virtual void getWindowSize(u32 *width, u32 *height) override;
	virtual void setWindowTitle(const wchar_t *title) override;
	virtual bool isActive() const override;
	virtual void showError(const wchar_t *error) override;
	virtual void showError(const char *error) override;
	virtual bool processMessage() override;

	virtual void setMouseVisible(bool isVisible) override;
	virtual bool isMouseVisible() const { return mouseVisible; }
	virtual void setMousePosition(int x, int y) override;

private:
	LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE hInstance = nullptr;
	HWND windowHandle = nullptr;
	int w, h;
	bool mouseVisible;
	u64 performanceFrequency;

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
