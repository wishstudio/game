#pragma once

class EventReceiver: public IEventReceiver
{
public:
	EventReceiver();
	virtual ~EventReceiver();

	virtual bool OnEvent(const SEvent &event) override;
	virtual bool isLeftButtonDown() const { return leftButtonDown; }
	virtual bool isRightButtonDown() const { return rightButtonDown; }
	virtual bool isKeyDown(EKEY_CODE keyCode) const {return keyIsDown[keyCode];}
	virtual f32 getMouseWheel() const {return mouseWheel;}
	virtual void resetWheel() {mouseWheel = 0.f;}

private:
	bool leftButtonDown;
	bool rightButtonDown;
	f32 mouseWheel;
	bool keyIsDown[KEY_KEY_CODES_COUNT];
};
