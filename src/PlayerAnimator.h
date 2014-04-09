#pragma once

class PlayerAnimator
{
public:
	PlayerAnimator();
	virtual ~PlayerAnimator();

	void tick(bool uiOpened);
	void update(bool uiOpened);

private:
	static float3 collideEllipsoidWithWorld(float3 currentPosition, float3 moveVector, bool canSlide, bool &collided);

	float3 currentPosition, nextPosition;
	float nextVerticalVelocity, nextVerticalDistance;
	bool nextFalling;
	float3 jumpDistance;

	float rotationHorizontal = 0, rotationVertical = 0;
};
