#pragma once

#include "InputValue.h"
#include "Vector2D.h"

class Device
{
public:
	Device();
	virtual ~Device() {}

	virtual bool init(u32 width, u32 height) = 0;
	virtual bool setWindowSize(u32 width, u32 height) = 0;
	virtual void getWindowSize(u32 *width, u32 *height) = 0;
	virtual void setWindowTitle(const wchar_t *title) = 0;
	virtual bool isActive() const = 0;
	virtual void showError(const wchar_t *error) = 0;
	virtual void showError(const char *error) = 0;
	virtual bool processMessage() = 0;

	/* Device input */
	bool isKeyDown(KeyValue key) const { return keyDownState[key]; }
	bool isKeyPressed(KeyValue key) const { return keyPressState[key]; }
	bool isMouseDown(MouseButton button) const { return mouseDownState[button]; }
	bool isMousePressed(MouseButton button) const { return mousePressState[button]; }
	f32 getMouseWheel() const { return mouseWheel; }

	virtual void setMouseVisible(bool isVisible) = 0;
	virtual bool isMouseVisible() const = 0;
	Vector2D getNormalizedMousePosition() const { return normalizedMousePosition; }
	virtual void setMousePosition(int x, int y) = 0;
	void setNormalizedMousePosition(f32 x, f32 y);
	void setNormalizedMousePosition(Vector2D position) { setNormalizedMousePosition(position.x, position.y); }

	/* Time flow */
	void setTicksPerSecond(u32 _ticksPerSecond) { ticksPerSecond = _ticksPerSecond; }
	bool tick();
	u64 getTick() const { return currentTick; }
	f32 getTickInterval() const { return 1.0f / ticksPerSecond; }
	u32 getElapsedTickTimeMilliseconds() const { return (currentTimeMicroseconds - currentTickTimeMicroseconds) / 1000; }
	f32 getElapsedTickTime() const { return getElapsedTickTimeMilliseconds() / 1000.f; };

	u32 getFPS() const { return averageFrameTimeMicroseconds == 0? 0: 1000000.f / averageFrameTimeMicroseconds; }
	f32 getAverageFrameTime() const { return averageFrameTimeMicroseconds / 1000000.f; }

protected:
	void onKeyDown(KeyValue key);
	void onKeyUp(KeyValue key);
	void onMouseDown(MouseButton button);
	void onMouseUp(MouseButton up);
	void onMouseMove(int x, int y);
	void onMouseWheel(s32 degree);
	void onLostFocus();
	void onNewFrame(u64 currentTimeMicroseconds);

private:
	Vector2D normalizedMousePosition = Vector2D(0, 0);
	bool keyDownState[KEY_VALUE_COUNT], keyPressState[KEY_VALUE_COUNT];
	bool mouseDownState[MOUSE_BUTTON_COUNT], mousePressState[MOUSE_BUTTON_COUNT];
	f32 mouseWheel;
	u32 ticksPerSecond;
	u64 currentTick = 0, initialTimeMicroseconds = -1;
	u64 currentTimeMicroseconds, currentTickTimeMicroseconds;

	/* Used for FPS Counting */
	u32 averageFrameTimeMicroseconds = 0;
	u32 fpsFrameCount = 0, fpsLastTimeMicroseconds = 0;
};
