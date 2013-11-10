#include "stdafx.h"

#include <DirectXMath.h>

#include "Chunk.h"
#include "PlayerAnimator.h"
#include "TimeManager.h"
#include "World.h"

static const Vector3D playerRadius = { 0.48f, 1.9f, 0.48f };
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
	Vector3D currentDistance = { 0, 0, 0 };
	f32 currentVerticalVelocity = nextVerticalVelocity;
	f32 currentVerticalDistance = nextVerticalDistance;
	currentPosition = nextPosition;
	bool currentFalling = nextFalling;

	/* Update next velocity */
	const f32 MOVE_SPEED = 6 * seconds<f32>(TICK_DURATION);
	if (!currentFalling)
	{
		Vector3D forwardVec = (camera->getLookAt() - camera->getPosition());
		forwardVec.y = 0;
		forwardVec = forwardVec.getNormalized();
		Vector3D leftVec = forwardVec.crossProduct({ 0, -1, 0 });

		if (windowSystem->isKeyDown(KEY_W))
			currentDistance += forwardVec * MOVE_SPEED;
		if (windowSystem->isKeyDown(KEY_S))
			currentDistance -= forwardVec * MOVE_SPEED;
		if (windowSystem->isKeyDown(KEY_A))
			currentDistance += leftVec * MOVE_SPEED;
		if (windowSystem->isKeyDown(KEY_D))
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
		if (windowSystem->isKeyDown(KEY_SPACE))
			nextVerticalVelocity = 9.8;
		else
			nextVerticalVelocity = 0;
	}
	nextVerticalDistance = (currentVerticalVelocity + nextVerticalVelocity) * seconds<f32>(TICK_DURATION) / 2;

	/* Collision detection */
	bool collided;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentDistance, true, collided);
	f32 originalY = nextPosition.y;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentVerticalDistance * Vector3D(0, 1, 0), false, collided);

	/* Check if we are stadning on the ground */
	nextFalling = !(currentVerticalDistance < EPSILON && collided);
}

Vector3D PlayerAnimator::collideEllipsoidWithWorld(Vector3D position, Vector3D moveVector, bool canSlide, bool &collided)
{
	if (moveVector.getLength() < EPSILON)
		return position;

	collided = false;

	/* Transform matrix */
	Matrix4 translation = Matrix4::inverseTranslation(position);
	Matrix4 scale = Matrix4::scale(1.f / playerRadius.x, 1.f / playerRadius.y, 1.f / playerRadius.z);

	Matrix4 transform = translation * scale;
	Matrix4 invTransform = transform.getInverse();

	/* Update next position */
	/* Gather relevant triangles for collision detection
	* In general, we need 2x2x2 chunks for collision detection.
	* For axis x, if our x coordinate is less than 50% of current chunk,
	* we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	* The same applies for axis y and z. */
	AABB box(position - playerRadius, position + playerRadius);
	box.merge(position + moveVector - playerRadius);
	box.merge(position + moveVector + playerRadius);

	/* Calculating base coordinates */
	f32 fx = position.x / CHUNK_SIZE;
	f32 fy = position.y / CHUNK_SIZE;
	f32 fz = position.z / CHUNK_SIZE;

	int basex = (int)floor(fx);
	int basey = (int)floor(fy);
	int basez = (int)floor(fz);

	basex = (fx - basex < .5f) ? basex - 1 : basex;
	basey = (fy - basey < .5f) ? basey - 1 : basey;
	basez = (fz - basez < .5f) ? basez - 1 : basez;

	/* Collect triangles */
	std::vector<Triangle3D> triangles;
	for (int x = 0; x <= 1; x++)
		for (int y = 0; y <= 1; y++)
			for (int z = 0; z <= 1; z++)
			{
				Chunk *chunk = world->getChunk(x + basex, y + basey, z + basez);
				int outCount;
				chunk->getTriangles(triangles, box, transform);
			}

	/* Transform vectors to ellipsoid coordinate system */
	moveVector = moveVector * scale;

	position = position * transform;
	/* From now on, nextPosition is transformed position, currentPosition is not transformed */
	/* Note the sphere is unit sphere */

	Vector3D vn = moveVector.getNormalized();
	f32 remainDistance = moveVector.getLength();
	for (int depth = 0; depth < 5; depth++) /* Do sliding at most 5 times */
	{
		/* Find colliding triangle */
		f32 minDistance = remainDistance; /* Don't count if colliding distance is larger than move distance */
		Vector3D minPlaneIntersection;

		Vector3D invertedVelocity(-vn);
		for (const Triangle3D triangle : triangles)
		{
			if (!triangle.isFrontFacing(vn))
				continue;

			Vector3D normal = triangle.getNormal().getInverted().getNormalized();

			/* Sphere intersection point
			 * The potential intersection point on the sphere
			 */
			Vector3D sphereIntersection = position + normal; /* We have unit sphere */

			/* Plane intersection point
			 * The potential intersection point on the plane the triangle reside on
			 */
			Vector3D planeIntersection;
			if (rayIntersectsPlane(Ray3D(sphereIntersection, vn), triangle, planeIntersection));
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
					if (!rayIntersectsWithSphere(Ray3D(planeIntersection, invertedVelocity), position, 1, distance))
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
		Vector3D snormal = (position - minPlaneIntersection).getNormalized();
		Vector3D remainVector = vn * remainDistance;
		Vector3D slideVector = remainVector + snormal * snormal.dotProduct(-remainVector);
		remainDistance = slideVector.getLength();
		vn = slideVector.getNormalized();
		if (remainDistance < veryCloseDistance)
			break;
	}
	/* Transform back to world coordinate system */
	position = position * invTransform;

	return position;
}

void PlayerAnimator::update()
{
	const Vector3D playerCameraOffset(0, 1.7f, 0);

	/* Interpolate camera position for smooth rendering */
	Vector3D position(currentPosition);
	Vector3D diffVec(nextPosition - currentPosition);
	position += diffVec * (seconds<f32>(timeManager->getRemainingTickDuration()) / seconds<f32>(TICK_DURATION));
	camera->setPosition(position + playerCameraOffset);
}
