#pragma once

class PlayerAnimator
{
public:
	PlayerAnimator();
	virtual ~PlayerAnimator();

	void tick();
	void update();

private:
	static vector3df collideEllipsoidWithWorld(vector3df currentPosition, vector3df moveVector, bool canSlide, bool &collided);

	vector3df currentPosition, nextPosition;
	f32 nextVerticalVelocity, nextVerticalDistance;
	bool nextFalling;
	vector3df forwardVec, leftVec, jumpDistance;
};
