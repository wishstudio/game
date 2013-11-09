#pragma once

class PlayerAnimator
{
public:
	PlayerAnimator();
	virtual ~PlayerAnimator();

	void tick();
	void update();

private:
	static Vector3 collideEllipsoidWithWorld(Vector3 currentPosition, Vector3 moveVector, bool canSlide, bool &collided);

	Vector3 currentPosition, nextPosition;
	f32 nextVerticalVelocity, nextVerticalDistance;
	bool nextFalling;
	Vector3 jumpDistance;
};
