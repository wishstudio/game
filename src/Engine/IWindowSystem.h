#pragma once

#include "InputValue.h"
#include "Vector2D.h"

class IWindowSystem
{
public:
	IWindowSystem();
	virtual ~IWindowSystem() {}

	virtual bool init(int width, int height) = 0;
	virtual bool setWindowSize(int width, int height) = 0;
	virtual void getWindowSize(int *width, int *height) = 0;
	virtual void setWindowTitle(const wchar_t *title) = 0;
	virtual bool isActive() const = 0;
	virtual void showError(const wchar_t *error) = 0;
	virtual void showError(const char *error) = 0;
	virtual bool processMessage() = 0;

	bool isKeyDown(KeyValue key) const { return keyDownState[key]; }
	bool isKeyPressed(KeyValue key) const { return keyPressState[key]; }
	bool isMouseDown(MouseButton button) const { return mouseDownState[button]; }
	bool isMousePressed(MouseButton button) const { return mousePressState[button]; }

	virtual void setMouseVisible(bool isVisible) = 0;
	virtual bool isMouseVisible() const = 0;
	Vector2D getNormalizedMousePosition() const { return normalizedMousePosition; }
	virtual void setMousePosition(int x, int y) = 0;
	void setNormalizedMousePosition(f32 x, f32 y);
	void setNormalizedMousePosition(Vector2D position) { setNormalizedMousePosition(position.x, position.y); }

protected:
	void onKeyDown(KeyValue key);
	void onKeyUp(KeyValue key);
	void onMouseDown(MouseButton button);
	void onMouseUp(MouseButton up);
	void onMouseMove(int x, int y);
	void onLostFocus();
	void onNewFrame();

private:
	Vector2D normalizedMousePosition = Vector2D(0, 0);
	bool keyDownState[KEY_VALUE_COUNT], keyPressState[KEY_VALUE_COUNT];
	bool mouseDownState[MOUSE_BUTTON_COUNT], mousePressState[MOUSE_BUTTON_COUNT];
};
