#pragma once

class EventReceiver: public irr::IEventReceiver
{
public:
	EventReceiver();
	virtual ~EventReceiver();

	virtual bool OnEvent(const irr::SEvent &event) override;
	
	void update();
	bool isLeftButtonDown() const { return leftButtonDown; }
	bool isRightButtonDown() const { return rightButtonDown; }
	bool isKeyDown(irr::EKEY_CODE keyCode) const { return keyIsDown[keyCode]; }
	bool isKeyPressed(irr::EKEY_CODE keyCode) const { return keyIsPressed[keyCode]; }
	f32 getMouseWheel();

private:
	bool leftButtonDown;
	bool rightButtonDown;
	f32 mouseWheel;
	bool keyIsDown[irr::KEY_KEY_CODES_COUNT], keyIsPressed[irr::KEY_KEY_CODES_COUNT];
};
