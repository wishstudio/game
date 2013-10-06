#pragma once

class EventReceiver: public IEventReceiver
{
public:
	EventReceiver();
	virtual ~EventReceiver();

	virtual bool OnEvent(const SEvent &event) override;
	virtual bool isLeftButtonDown() const { return leftButtonDown; }
	virtual bool isRightButtonDown() const { return rightButtonDown; }

private:
	bool leftButtonDown;
	bool rightButtonDown;
};
