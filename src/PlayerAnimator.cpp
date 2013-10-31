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

	//nextVelocity += { 0, -1, 0 };

	/* Update next position */
	const f32 playerRadius = .5f;
	vector3df moveVector = currentVelocity * seconds<f32>(TICK_DURATION);
	nextPosition = currentPosition + moveVector;
	aabbox3df box(nextPosition - playerRadius, nextPosition + playerRadius);
	box.addInternalBox(aabbox3df(currentPosition - playerRadius, currentPosition + playerRadius));
	box.MinEdge -= { 1, 1, 1 };
	box.MaxEdge += { 1, 1, 1 };

	/* Gather relevant triangles for collision detection
	In general, we need 2x2x2 chunks for collision detection.
	For axis x, if our x coordinate is less than 50% of current chunk,
	we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	The same applies for axis y and z. */

	/* Calculating base coordinates */
	f32 fx = nextPosition.X / CHUNK_SIZE;
	f32 fy = nextPosition.Y / CHUNK_SIZE;
	f32 fz = nextPosition.Z / CHUNK_SIZE;

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

	/* Find colliding triangle */
	f32 minDistance = moveVector.getLength(); /* Don't count if colliding distance is larger than move distance */
	int minTriangleId = -1;
	vector3df invertedVelocity(currentVelocity);
	invertedVelocity.invert();
	for (int i = 0; i < count; i++)
	{
		if (!triangles[i].isFrontFacing(currentVelocity))
			continue;

		vector3df normal = triangles[i].getNormal().invert().normalize();

		/* Sphere intersection point */
		vector3df sphereIntersection = currentPosition + normal * playerRadius;

		/* Plane intersection point */
		vector3df planeIntersection;
		if (triangles[i].getIntersectionOfPlaneWithLine(sphereIntersection, currentVelocity, planeIntersection))
		{
			f32 distance;
			if (triangles[i].isPointInside(planeIntersection))
			{
				/* Simple case: plane intersection in on the triangle */
				distance = (planeIntersection - sphereIntersection).getLength();
			}
			else
			{
				/* Hard case: reverse intersecting the sphere */
				vector3df closest = triangles[i].closestPointOnTriangle(planeIntersection);

				if (!rayIntersectsWithSphere(closest, invertedVelocity, currentPosition, playerRadius, distance))
					continue;
			}

			if (distance < minDistance)
			{
				minDistance = distance;
				minTriangleId = i;
			}
		}
	}
	delete triangles;

	if (minTriangleId == -1) /* No collision */
		return;
	
	/* Simple treat now */
	vector3df vn(currentVelocity);
	vn.normalize();
	nextPosition = currentPosition + vn * minDistance;
}

void PlayerAnimator::update()
{
	vector3df position(currentPosition);
	vector3df diffVec(nextPosition - currentPosition);
	position += diffVec * (seconds<f32>(timeManager->getRemainingTickDuration()) / seconds<f32>(TICK_DURATION));
	camera->setPosition(position);
}
