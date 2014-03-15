#pragma once

class PlayerAnimator
{
public:
	PlayerAnimator();
	virtual ~PlayerAnimator();

	void tick();
	void update();

private:
	static Vector3D collideEllipsoidWithWorld(Vector3D currentPosition, Vector3D moveVector, bool canSlide, bool &collided);

	Vector3D currentPosition, nextPosition;
	float nextVerticalVelocity, nextVerticalDistance;
	bool nextFalling;
	Vector3D jumpDistance;

	float rotationHorizontal = 0, rotationVertical = 0;
};
