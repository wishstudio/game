#include "stdafx.h"

#include "Chunk.h"
#include "EventReceiver.h"
#include "PlayerAnimator.h"
#include "TimeManager.h"
#include "World.h"

static const vector3df playerRadius = { 0.48f, 1.9f, 0.48f };
static const f32 veryCloseDistance = 0.0001;

PlayerAnimator::PlayerAnimator()
{
	nextPosition = { 0, 100, 0 };
	nextVerticalVelocity = 0;
	nextFalling = true;
}

PlayerAnimator::~PlayerAnimator()
{
}

void PlayerAnimator::tick()
{
	/* Update current values */
	vector3df currentDistance = { 0, 0, 0 };
	f32 currentVerticalVelocity = nextVerticalVelocity;
	f32 currentVerticalDistance = nextVerticalDistance;
	currentPosition = nextPosition;
	bool currentFalling = nextFalling;

	/* Update next velocity */
	const f32 MOVE_SPEED = 6 * seconds<f32>(TICK_DURATION);
	if (!currentFalling)
	{
		vector3df forwardVec = (camera->getTarget() - camera->getPosition());
		forwardVec.Y = 0;
		forwardVec.normalize();
		vector3df leftVec = forwardVec.crossProduct({ 0, 1, 0 });

		if (eventReceiver->isKeyDown(KEY_KEY_W))
			currentDistance += forwardVec * MOVE_SPEED;
		if (eventReceiver->isKeyDown(KEY_KEY_S))
			currentDistance -= forwardVec * MOVE_SPEED;
		if (eventReceiver->isKeyDown(KEY_KEY_A))
			currentDistance += leftVec * MOVE_SPEED;
		if (eventReceiver->isKeyDown(KEY_KEY_D))
			currentDistance -= leftVec * MOVE_SPEED;

		jumpDistance = currentDistance;
	}
	else
		currentDistance = jumpDistance;

	if (currentFalling)
		/* Add gravity */
		nextVerticalVelocity -= 9.8 * 2 * seconds<f32>(TICK_DURATION);
	else
	{
		if (eventReceiver->isKeyDown(KEY_SPACE))
			nextVerticalVelocity = 9.8;
		else
			nextVerticalVelocity = 0;
	}
	nextVerticalDistance = (currentVerticalVelocity + nextVerticalVelocity) * seconds<f32>(TICK_DURATION) / 2;

	/* Collision detection */
	bool collided;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentDistance, true, collided);
	f32 originalY = nextPosition.Y;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentVerticalDistance * vector3df(0, 1, 0), false, collided);

	/* Check if we are stadning on the ground */
	nextFalling = !(currentVerticalDistance < ROUNDING_ERROR_f32 && collided);
}

vector3df PlayerAnimator::collideEllipsoidWithWorld(vector3df position, vector3df moveVector, bool canSlide, bool &collided)
{
	collided = false;

	/* Transform matrix */
	matrix4 translation = matrix4(matrix4::EM4CONST_IDENTITY).setInverseTranslation(position);
	matrix4 scale = matrix4(matrix4::EM4CONST_IDENTITY);
	scale.setScale({ 1.f / playerRadius.X, 1.f / playerRadius.Y, 1.f / playerRadius.Z });

	matrix4 transform = scale * translation;
	matrix4 invTransform(matrix4::EM4CONST_NOTHING);
	transform.getInverse(invTransform);

	/* Update next position */
	/* Gather relevant triangles for collision detection
	* In general, we need 2x2x2 chunks for collision detection.
	* For axis x, if our x coordinate is less than 50% of current chunk,
	* we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	* The same applies for axis y and z. */
	aabbox3df box(position - playerRadius, position + playerRadius);
	box.addInternalPoint(position + moveVector - playerRadius);
	box.addInternalPoint(position + moveVector + playerRadius);

	/* Calculating base coordinates */
	f32 fx = position.X / CHUNK_SIZE;
	f32 fy = position.Y / CHUNK_SIZE;
	f32 fz = position.Z / CHUNK_SIZE;

	int basex = (int)floor(fx);
	int basey = (int)floor(fy);
	int basez = (int)floor(fz);

	basex = (fx - basex < .5f) ? basex - 1 : basex;
	basey = (fy - basey < .5f) ? basey - 1 : basey;
	basez = (fz - basez < .5f) ? basez - 1 : basez;

	/* Collect triangles */
	std::vector<triangle3df> triangles;
	for (int x = 0; x <= 1; x++)
	for (int y = 0; y <= 1; y++)
	for (int z = 0; z <= 1; z++)
	{
		Chunk *chunk = world->getChunk(x + basex, y + basey, z + basez);
		int outCount;
		chunk->getTriangles(triangles, box, transform);
	}

	/* Transform vectors to ellipsoid coordinate system */
	scale.transformVect(moveVector);
	transform.transformVect(position);
	/* From now on, nextPosition is transformed position, currentPosition is not transformed */
	/* Note the sphere is unit sphere */

	vector3df vn(moveVector);
	vn.normalize();
	f32 remainDistance = moveVector.getLength();
	for (int depth = 0; depth < 5; depth++) /* Do sliding at most 5 times */
	{
		/* Find colliding triangle */
		f32 minDistance = remainDistance; /* Don't count if colliding distance is larger than move distance */
		vector3df minPlaneIntersection;

		vector3df invertedVelocity(vn);
		invertedVelocity.invert();
		for (const triangle3df triangle : triangles)
		{
			if (!triangle.isFrontFacing(vn))
				continue;

			vector3df normal = triangle.getNormal().invert().normalize();

			/* Sphere intersection point
			 * The potential intersection point on the sphere
			 */
			vector3df sphereIntersection = position + normal; /* We have unit sphere */

			/* Plane intersection point
			 * The potential intersection point on the plane the triangle reside on
			 */
			vector3df planeIntersection;
			if (triangle.getIntersectionOfPlaneWithLine(sphereIntersection, vn, planeIntersection))
			{
				f32 distance;
				if (triangle.isPointInside(planeIntersection))
				{
					/* Simple case: plane intersection in on the triangle */
					distance = (planeIntersection - sphereIntersection).getLength();
				}
				else
				{
					/* Hard case: plane intersection is not on the triangle
					 * Will intersect at the point nearest to the plane intersection point */
					planeIntersection = triangle.closestPointOnTriangle(planeIntersection);

					/* Reverse intersecting the sphere */
					if (!rayIntersectsWithSphere(planeIntersection, invertedVelocity, position, 1, distance))
						continue;
				}

				if (distance <= minDistance)
				{
					collided = true;
					minDistance = distance;
					minPlaneIntersection = planeIntersection;
				}
			}
		}
		/* Move */
		/* Do not touch the triangle exactly for a tolerance of floating point errors */
		position += vn * (minDistance - veryCloseDistance);
		remainDistance -= (minDistance - veryCloseDistance);
		minPlaneIntersection -= vn * veryCloseDistance;

		if (remainDistance < veryCloseDistance)
			break;

		if (!canSlide)
			break;

		/* Sliding plane
		 * Origin is the point of plane intersection
		 * Normal is from the intersection point to the center of the sphere
		 */
		vector3df snormal = (position - minPlaneIntersection).normalize();
		vector3df remainVector = vn * remainDistance;
		vector3df slideVector = remainVector + snormal * snormal.dotProduct(-remainVector);
		remainDistance = slideVector.getLength();
		vn = slideVector.normalize();
		if (remainDistance < veryCloseDistance)
			break;
	}
	/* Transform back to world coordinate system */
	invTransform.transformVect(position);

	return position;
}

void PlayerAnimator::update()
{
	const vector3df playerCameraOffset(0, 1.7f, 0);

	/* Interpolate camera position for smooth rendering */
	vector3df position(currentPosition);
	vector3df diffVec(nextPosition - currentPosition);
	position += diffVec * (seconds<f32>(timeManager->getRemainingTickDuration()) / seconds<f32>(TICK_DURATION));
	camera->setPosition(position + playerCameraOffset);
}
