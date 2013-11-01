#include "stdafx.h"

#include "Chunk.h"
#include "EventReceiver.h"
#include "PlayerAnimator.h"
#include "TimeManager.h"
#include "World.h"

PlayerAnimator::PlayerAnimator()
{
	nextPosition = { 0, 10, 0 };
	nextVelocity = { 0, 0, 0 };
}

PlayerAnimator::~PlayerAnimator()
{
}

void PlayerAnimator::tick()
{
	/* Player definition */
	const vector3df playerRadius = { 0.48f, 1.9f, 0.48f };

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
		nextVelocity -= forwardVec * MOVE_SPEED;
	if (eventReceiver->isKeyDown(KEY_KEY_A))
		nextVelocity += leftVec * MOVE_SPEED;
	if (eventReceiver->isKeyDown(KEY_KEY_D))
		nextVelocity -= leftVec * MOVE_SPEED;
	
	/* Add gravity */
	nextVelocity += vector3df(0, -100, 0) * seconds<f32>(TICK_DURATION);


	/* Update next position */
	/* Gather relevant triangles for collision detection
	 * In general, we need 2x2x2 chunks for collision detection.
	 * For axis x, if our x coordinate is less than 50% of current chunk,
	 * we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	 * The same applies for axis y and z. */
	vector3df moveVector = currentVelocity * seconds<f32>(TICK_DURATION);
	aabbox3df box(currentPosition - playerRadius, currentPosition + playerRadius);
	box.addInternalPoint(currentPosition + moveVector - playerRadius);
	box.addInternalPoint(currentPosition + moveVector + playerRadius);

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

	/* Transform matrix */
	matrix4 translation = matrix4(matrix4::EM4CONST_IDENTITY).setInverseTranslation(currentPosition);
	matrix4 scale = matrix4(matrix4::EM4CONST_IDENTITY);
	scale.setScale({ 1.f / playerRadius.X, 1.f / playerRadius.Y, 1.f / playerRadius.Z });

	matrix4 transform = scale * translation;
	matrix4 invTransform(matrix4::EM4CONST_NOTHING);
	transform.getInverse(invTransform);

	/* Collect triangles */
	const int ARRAY_SIZE = 65536;
	auto triangles = new triangle3df[ARRAY_SIZE];
	int count = 0;
	for (int x = 0; x <= 1; x++)
		for (int y = 0; y <= 1; y++)
			for (int z = 0; z <= 1; z++)
			{
				Chunk *chunk = world->getChunk(x + basex, y + basey, z + basez);
				int outCount;
				chunk->getTriangles(triangles + count, ARRAY_SIZE - count, outCount, box, &transform);
				count += outCount;
			}

	/* Transform vectors to ellipsoid coordinate system */
	scale.transformVect(moveVector);
	transform.transformVect(nextPosition);
	/* From now on, nextPosition is transformed position, currentPosition is not transformed */
	/* Note the sphere is unit sphere */

	/* Collision update loop */
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
		for (int i = 0; i < count; i++)
		{
			if (!triangles[i].isFrontFacing(vn))
				continue;

			vector3df normal = triangles[i].getNormal().invert().normalize();

			/* Sphere intersection point
			 * The potential intersection point on the sphere
			 */
			vector3df sphereIntersection = nextPosition + normal; /* We have unit sphere */

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
					if (!rayIntersectsWithSphere(planeIntersection, invertedVelocity, nextPosition, 1, distance))
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
		/* Do not touch the triangle exactly for a tolerance of floating point errors */
		const f32 veryCloseDistance = 0.0001;
		nextPosition += vn * (minDistance - veryCloseDistance);
		remainDistance -= (minDistance - veryCloseDistance);
		minPlaneIntersection -= vn * veryCloseDistance;

		if (remainDistance < veryCloseDistance)
			break;
		
		/* Sliding plane
		 * Origin is the point of plane intersection
		 * Normal is from the intersection point to the center of the sphere
		 */
		vector3df snormal = (nextPosition - minPlaneIntersection).normalize();
		vector3df remainVector = vn * remainDistance;
		vector3df slideVector = remainVector + snormal * snormal.dotProduct(-remainVector);
		remainDistance = slideVector.getLength();
		vn = slideVector.normalize();
		if (remainDistance < veryCloseDistance)
			break;
	}
	delete triangles;

	/* Transform back to world coordinate system */
	invTransform.transformVect(nextPosition);
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
