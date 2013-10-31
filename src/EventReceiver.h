#pragma once

class EventReceiver: public IEventReceiver
{
public:
	EventReceiver();
	virtual ~EventReceiver();

	virtual bool OnEvent(const SEvent &event) override;
	
	void update();
	bool isLeftButtonDown() const { return leftButtonDown; }
	bool isRightButtonDown() const { return rightButtonDown; }
	bool isKeyDown(EKEY_CODE keyCode) const { return keyIsDown[keyCode]; }
	bool isKeyPressed(EKEY_CODE keyCode) const { return keyIsPressed[keyCode]; }
	f32 getMouseWheel();

private:
	bool leftButtonDown;
	bool rightButtonDown;
	f32 mouseWheel;
	bool keyIsDown[KEY_KEY_CODES_COUNT], keyIsPressed[KEY_KEY_CODES_COUNT];
};
