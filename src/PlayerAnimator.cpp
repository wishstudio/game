#include "stdafx.h"

#include "Chunk.h"
#include "EventReceiver.h"
#include "PlayerAnimator.h"
#include "TimeManager.h"
#include "World.h"

PlayerAnimator::PlayerAnimator()
{
	nextPosition = { 0, 5, 0 };
	nextVelocity = { 0, 0, 0 };
}

PlayerAnimator::~PlayerAnimator()
{
}

void PlayerAnimator::tick()
{
	/* Update current values */
	vector3df currentVelocity = nextVelocity;
	currentPosition = nextPosition;

	/* Update next velocity */
	const int MOVE_SPEED = 10;
	nextVelocity = { 0, 0, 0 };
	vector3df forwardVec(camera->getTarget() - camera->getPosition());
	forwardVec.Y = 0;
	forwardVec.normalize();
	vector3df leftVec = forwardVec.crossProduct({ 0, 1, 0 });

	if (eventReceiver->isKeyDown(KEY_KEY_W))
		nextVelocity += forwardVec * MOVE_SPEED;
	if (eventReceiver->isKeyDown(KEY_KEY_S))
		nextVelocity += forwardVec * -MOVE_SPEED;
	if (eventReceiver->isKeyDown(KEY_KEY_A))
		nextVelocity += leftVec * MOVE_SPEED;
	if (eventReceiver->isKeyDown(KEY_KEY_D))
		nextVelocity += leftVec * -MOVE_SPEED;

	nextVelocity += { 0, -1, 0 };

	/* Update next position */
	const f32 playerRadius = .5f;
	const vector3df moveVector = currentVelocity * seconds<f32>(TICK_DURATION);
	aabbox3df box(currentPosition - moveVector.getLength() - playerRadius);
	box.addInternalPoint(currentPosition + moveVector.getLength() + playerRadius);

	/* Gather relevant triangles for collision detection
	In general, we need 2x2x2 chunks for collision detection.
	For axis x, if our x coordinate is less than 50% of current chunk,
	we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	The same applies for axis y and z. */

	/* Calculating base coordinates */
	f32 fx = currentPosition.X / CHUNK_SIZE;
	f32 fy = currentPosition.Y / CHUNK_SIZE;
	f32 fz = currentPosition.Z / CHUNK_SIZE;

	int basex = (int) floor(fx);
	int basey = (int) floor(fy);
	int basez = (int) floor(fz);

	basex = (fx - basex < .5f) ? basex - 1 : basex;
	basey = (fy - basey < .5f) ? basey - 1 : basey;
	basez = (fz - basez < .5f) ? basez - 1 : basez;

	const int ARRAY_SIZE = 65536;
	auto triangles = new triangle3df[ARRAY_SIZE];
	int count = 0;

	/* Collect triangles */
	for (int x = 0; x <= 1; x++)
		for (int y = 0; y <= 1; y++)
			for (int z = 0; z <= 1; z++)
			{
				Chunk *chunk = world->getChunk(x + basex, y + basey, z + basez);
				int outCount;
				chunk->getTriangles(triangles + count, ARRAY_SIZE - count, outCount, box);
				count += outCount;
			}

	/* Collision update loop */
	vector3df vn(currentVelocity);
	vn.normalize();
	f32 remainDistance = moveVector.getLength();
	while (remainDistance > ROUNDING_ERROR_f32)
	{
		/* Find colliding triangle */
		f32 minDistance = remainDistance; /* Don't count if colliding distance is larger than move distance */
		vector3df minPlaneIntersection;

		vector3df invertedVelocity(vn);
		invertedVelocity.invert();
		for (int i = 0; i < count; i++)
		{
			if (!triangles[i].isFrontFacing(vn))
				continue;

			vector3df normal = triangles[i].getNormal().invert().normalize();

			/* Sphere intersection point
			 * The potential intersection point on the sphere
			 */
			vector3df sphereIntersection = nextPosition + normal * playerRadius;

			/* Plane intersection point
			 * The potential intersection point on the plane the triangle reside on
			 */
			vector3df planeIntersection;
			if (triangles[i].getIntersectionOfPlaneWithLine(sphereIntersection, vn, planeIntersection))
			{
				f32 distance;
				if (triangles[i].isPointInside(planeIntersection))
				{
					/* Simple case: plane intersection in on the triangle */
					distance = (planeIntersection - sphereIntersection).getLength();
				}
				else
				{
					/* Hard case: plane intersection is not on the triangle
					 * Will intersect at the point nearest to the plane intersection point */
					planeIntersection = triangles[i].closestPointOnTriangle(planeIntersection);

					/* Reverse intersecting the sphere */
					if (!rayIntersectsWithSphere(planeIntersection, invertedVelocity, nextPosition, playerRadius, distance))
						continue;
				}

				if (distance < minDistance)
				{
					minDistance = distance;
					minPlaneIntersection = planeIntersection;
				}
			}
		}
		/* Move */
		nextPosition += vn * minDistance;
		remainDistance -= minDistance;

		if (remainDistance > ROUNDING_ERROR_f32)
		{
			/* Sliding plane
			 * Origin is the point of plane intersection
			 * Normal is from the intersection point to the center of the sphere
			 */
			vector3df snormal = (nextPosition - minPlaneIntersection).normalize();
			vector3df remainVector = vn * remainDistance;
			vector3df slideVector = remainVector + snormal * snormal.dotProduct(-remainVector);
			remainDistance = slideVector.getLength();
			vn = slideVector.normalize();
		}
	}
	delete triangles;
}

void PlayerAnimator::update()
{
	vector3df position(currentPosition);
	vector3df diffVec(nextPosition - currentPosition);
	position += diffVec * (seconds<f32>(timeManager->getRemainingTickDuration()) / seconds<f32>(TICK_DURATION));
	camera->setPosition(position);
}
