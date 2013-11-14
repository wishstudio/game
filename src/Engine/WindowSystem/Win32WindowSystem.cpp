#include <stdafx.h>

#include <windowsx.h>

#include "Win32WindowSystem.h"

static KeyValue Win32KeyCodes[256] = {
			/* 0x00 */		/* 0x01 */		/* 0x02 */		/* 0x03 */		/* 0x04 */		/* 0x05	*/		/* 0x06 */		/* 0x07 */
			/* 0x08 */		/* 0x09 */		/* 0x0A */		/* 0x0B */		/* 0x0C */		/* 0x0D	*/		/* 0x0E */		/* 0x0F */

/* 0x00 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_BACKSPACE,	KEY_TAB,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_ENTER,		KEY_NONE,		KEY_NONE,

/* 0x10 */	KEY_SHIFT,		KEY_CTRL,		KEY_ALT,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_ESC,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0x20 */	KEY_SPACE,		KEY_PAGEUP,		KEY_PAGEDOWN,	KEY_END,		KEY_HOME,		KEY_LEFT,		KEY_UP,			KEY_RIGHT,
			KEY_DOWN,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_INSERT,		KEY_DELETE,		KEY_NONE,

/* 0x30 */	KEY_0,			KEY_1,			KEY_2,			KEY_3,			KEY_4,			KEY_5,			KEY_6,			KEY_7,
			KEY_8,			KEY_9,			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0x40 */	KEY_NONE,		KEY_A,			KEY_B,			KEY_C,			KEY_D,			KEY_E,			KEY_F,			KEY_G,
			KEY_H,			KEY_I,			KEY_J,			KEY_K,			KEY_L,			KEY_M,			KEY_N,			KEY_O,

/* 0x50 */	KEY_P,			KEY_Q,			KEY_R,			KEY_S,			KEY_T,			KEY_U,			KEY_V,			KEY_W,
			KEY_X,			KEY_Y,			KEY_Z,			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0x60 */	KEY_NUMPAD_0,	KEY_NUMPAD_1,	KEY_NUMPAD_2,	KEY_NUMPAD_3,	KEY_NUMPAD_4,	KEY_NUMPAD_5,	KEY_NUMPAD_6,	KEY_NUMPAD_7,
			KEY_NUMPAD_8,	KEY_NUMPAD_9,	KEY_NUMPAD_MUL,	KEY_NUMPAD_ADD,	KEY_NONE,		KEY_NUMPAD_SUB,	KEY_NUMPAD_DOT,	KEY_NUMPAD_DIV,

/* 0x70 */	KEY_F1,			KEY_F2,			KEY_F3,			KEY_F4,			KEY_F5,			KEY_F6,			KEY_F7,			KEY_F8,
			KEY_F9,			KEY_F10,		KEY_F11,		KEY_F12,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0x80 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0x90 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0xA0 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0xB0 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_COLON,		KEY_ADD,		KEY_COMMA,		KEY_SUB,		KEY_DOT,		KEY_QUESTION,

/* 0xC0 */	KEY_WAVE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0xD0 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_LBRACKET,	KEY_LINE,		KEY_RBRACKET,	KEY_QUOTE,		KEY_NONE,

/* 0xE0 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,

/* 0xF0 */	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
			KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32WindowSystem *windowSystem = (Win32WindowSystem *) GetWindowLongW(hWnd, GWLP_USERDATA);
	return windowSystem->wndProc(hWnd, message, wParam, lParam);
}

Win32WindowSystem::Win32WindowSystem()
{
	hInstance = GetModuleHandleW(nullptr);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	performanceFrequency = frequency.QuadPart;
}

Win32WindowSystem::~Win32WindowSystem()
{
}

bool Win32WindowSystem::init(int width, int height)
{
	WNDCLASSEXW wcex;
	wcex.cbSize             = sizeof(WNDCLASSEX);
	wcex.style              = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc        = WndProc;
	wcex.cbClsExtra         = 0;
	wcex.cbWndExtra         = 0;
	wcex.hInstance          = hInstance;
	wcex.hIcon              = 0;
	wcex.hCursor            = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground      = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName       = nullptr;
	wcex.lpszClassName      = getWindowClassName();
	wcex.hIconSm            = 0;
	RegisterClassExW(&wcex);

	/* Create the window */
	windowHandle = CreateWindowExW(0, getWindowClassName(), L"",
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, hInstance, nullptr);

	if (windowHandle == nullptr)
		return false;
	SetWindowLongW(windowHandle, GWLP_USERDATA, (LONG) this);
	setWindowSize(width, height);

	/* Show the window */
	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);
	return true;
}

bool Win32WindowSystem::setWindowSize(int width, int height)
{
	w = width, h = height;
	if (width == 0 && height == 0)
		SetWindowLong(windowHandle, GWL_STYLE, GetWindowLong(windowHandle, GWL_STYLE) | WS_POPUP | WS_EX_TOPMOST);
	else
	{
		SetWindowLong(windowHandle, GWL_STYLE, GetWindowLong(windowHandle, GWL_STYLE) & ~(WS_POPUP | WS_EX_TOPMOST));

		RECT r;
		GetWindowRect(windowHandle, &r);
		/* Convert client size to window size */
		RECT rect;
		rect.top = 0;
		rect.left = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

		MoveWindow(windowHandle, r.left, r.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
	}
	return true;
}

void Win32WindowSystem::getWindowSize(int *width, int *height)
{
	*width = w;
	*height = h;
}

void Win32WindowSystem::setWindowTitle(const wchar_t *title)
{
	SetWindowTextW(windowHandle, title);
}

bool Win32WindowSystem::isActive() const
{
	return GetActiveWindow() == windowHandle;
}

void Win32WindowSystem::showError(const wchar_t *error)
{
	MessageBoxW(windowHandle, error, L"ERROR", MB_OK | MB_ICONERROR);
}

void Win32WindowSystem::showError(const char *error)
{
	MessageBoxA(windowHandle, error, "ERROR", MB_OK | MB_ICONERROR);
}

bool Win32WindowSystem::processMessage()
{
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter(&currentCount);
	u64 currentTimeMicroseconds = ((double) currentCount.QuadPart / (double) performanceFrequency) * 1000000;
	onNewFrame(currentTimeMicroseconds);
	MSG msg;
	ZeroMemory(&msg, sizeof msg);
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}
	return true;
}

void Win32WindowSystem::setMouseVisible(bool isVisible)
{
	ShowCursor(isVisible);
	mouseVisible = isVisible;
}

void Win32WindowSystem::setMousePosition(int x, int y)
{
	RECT rect;
	GetClientRect(windowHandle, &rect);
	POINT topLeft;
	topLeft.x = rect.left;
	topLeft.y = rect.top;
	ClientToScreen(windowHandle, &topLeft);
	SetCursorPos(topLeft.x + x, topLeft.y + y);
}

LRESULT Win32WindowSystem::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 1;

	case WM_SYSKEYDOWN:
		if (wParam == VK_F10) /* Special case */
		{
			onKeyDown(KEY_F10);
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_F10)
		{
			onKeyUp(KEY_F10);
			return 0;
		}
		break;

	case WM_KEYDOWN:
		onKeyDown(Win32KeyCodes[wParam]);
		break;

	case WM_KEYUP:
		onKeyUp(Win32KeyCodes[wParam]);
		break;

	case WM_LBUTTONDOWN:
		onMouseDown(MOUSE_BUTTON_LEFT);
		break;

	case WM_LBUTTONUP:
		onMouseUp(MOUSE_BUTTON_LEFT);
		break;

	case WM_MBUTTONDOWN:
		onMouseDown(MOUSE_BUTTON_MIDDLE);
		break;

	case WM_MBUTTONUP:
		onMouseUp(MOUSE_BUTTON_MIDDLE);
		break;

	case WM_RBUTTONDOWN:
		onMouseDown(MOUSE_BUTTON_RIGHT);
		break;

	case WM_RBUTTONUP:
		onMouseUp(MOUSE_BUTTON_RIGHT);
		break;

	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
		onMouseMove(x, y);
		break;
	}

	case WM_KILLFOCUS:
		onLostFocus();
		break;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
