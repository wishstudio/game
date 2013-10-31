#pragma once

class PlayerAnimator
{
public:
	PlayerAnimator();
	virtual ~PlayerAnimator();

	void tick();
	void update();

private:
	vector3df currentPosition, nextPosition;
	vector3df nextVelocity;
};
