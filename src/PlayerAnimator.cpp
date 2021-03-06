#include "stdafx.h"

#include "Chunk.h"
#include "PlayerAnimator.h"
#include "World.h"

static const float3 playerRadius = { 0.48f, 1.9f, 0.48f };
static const float veryCloseDistance = 0.0001;

PlayerAnimator::PlayerAnimator()
{
	nextPosition = { 0, 10, 0 };
	nextVerticalVelocity = 0;
	nextFalling = true;
}

PlayerAnimator::~PlayerAnimator()
{
}

void PlayerAnimator::tick(bool uiOpened)
{
	/* Update current values */
	float3 currentDistance = { 0, 0, 0 };
	float currentVerticalVelocity = nextVerticalVelocity;
	float currentVerticalDistance = nextVerticalDistance;
	currentPosition = nextPosition;
	bool currentFalling = nextFalling;

	/* Update next velocity */
	const float MOVE_SPEED = 6 * device->getTickInterval();
	if (!currentFalling)
	{
		float3 forwardVec = (camera->getLookAt() - camera->getPosition());
		forwardVec.y = 0;
		forwardVec = forwardVec.getNormalized();
		float3 leftVec = forwardVec.crossProduct({ 0, -1, 0 });

		if (!uiOpened)
		{
			if (device->isKeyDown(KEY_W))
				currentDistance += forwardVec * MOVE_SPEED;
			if (device->isKeyDown(KEY_S))
				currentDistance -= forwardVec * MOVE_SPEED;
			if (device->isKeyDown(KEY_A))
				currentDistance += leftVec * MOVE_SPEED;
			if (device->isKeyDown(KEY_D))
				currentDistance -= leftVec * MOVE_SPEED;
		}

		jumpDistance = currentDistance;
	}
	else
		currentDistance = jumpDistance;

	if (currentFalling)
		/* Add gravity */
		nextVerticalVelocity -= 9.8 * 2 * device->getTickInterval();
	else
	{
		if (!uiOpened && device->isKeyDown(KEY_SPACE))
			nextVerticalVelocity = 9.8;
		else
			nextVerticalVelocity = 0;
	}
	nextVerticalDistance = (currentVerticalVelocity + nextVerticalVelocity) * device->getTickInterval() / 2;

	/* Collision detection */
	bool collided;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentDistance, true, collided);
	float originalY = nextPosition.y;
	nextPosition = collideEllipsoidWithWorld(nextPosition, currentVerticalDistance * float3(0, 1, 0), false, collided);

	/* Check if we are stadning on the ground */
	nextFalling = !(currentVerticalDistance < EPSILON && collided);
}

float3 PlayerAnimator::collideEllipsoidWithWorld(float3 position, float3 moveVector, bool canSlide, bool &collided)
{
	if (moveVector.getLength() < EPSILON)
		return position;

	collided = false;

	/* Transform matrix */
	float4x4 translation = float4x4::inverseTranslation(position);
	float4x4 scale = float4x4::scale(1.f / playerRadius.x, 1.f / playerRadius.y, 1.f / playerRadius.z);

	float4x4 transform = translation * scale;
	float4x4 invTransform = transform.getInversed();

	/* Update next position */
	/* Gather relevant triangles for collision detection
	* In general, we need 2x2x2 chunks for collision detection.
	* For axis x, if our x coordinate is less than 50% of current chunk,
	* we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	* The same applies for axis y and z. */
	AABB3D box(position - playerRadius, position + playerRadius);
	box.merge(position + moveVector - playerRadius);
	box.merge(position + moveVector + playerRadius);

	/* Calculating base coordinates */
	float fx = position.x / CHUNK_SIZE;
	float fy = position.y / CHUNK_SIZE;
	float fz = position.z / CHUNK_SIZE;

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

	float3 vn = moveVector.getNormalized();
	float remainDistance = moveVector.getLength();
	for (int depth = 0; depth < 5; depth++) /* Do sliding at most 5 times */
	{
		/* Find colliding triangle */
		float minDistance = remainDistance; /* Don't count if colliding distance is larger than move distance */
		float3 minPlaneIntersection;

		float3 invertedVelocity(-vn);
		for (const Triangle3D triangle : triangles)
		{
			if (!triangle.isFrontFacing(vn))
				continue;

			float3 normal = triangle.getNormal().getInverted().getNormalized();

			/* Sphere intersection point
			 * The potential intersection point on the sphere
			 */
			float3 sphereIntersection = position + normal; /* We have unit sphere */

			/* Plane intersection point
			 * The potential intersection point on the plane the triangle reside on
			 */
			float3 planeIntersection;
			if (rayIntersectsPlane(Ray3D(sphereIntersection, vn), triangle, planeIntersection));
			{
				float distance;
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
		float3 snormal = (position - minPlaneIntersection).getNormalized();
		float3 remainVector = vn * remainDistance;
		float3 slideVector = remainVector + snormal * snormal.dotProduct(-remainVector);
		remainDistance = slideVector.getLength();
		vn = slideVector.getNormalized();
		if (remainDistance < veryCloseDistance)
			break;
	}
	/* Transform back to world coordinate system */
	position = position * invTransform;

	return position;
}

void PlayerAnimator::update(bool uiOpened)
{
	const float3 playerCameraOffset(0, 1.7f, 0);
	const float VIEW_DELTA = 0.01;

	/* Interpolate camera position for smooth rendering */
	float3 position(currentPosition);
	float3 diffVec(nextPosition - currentPosition);
	position += diffVec * (device->getElapsedTickTime() / device->getTickInterval());
	position += playerCameraOffset;
	camera->setPosition(position);

	if (!uiOpened)
	{
		/* Update rotation */
		float2 mouseDelta = device->getNormalizedMousePosition();
		/* TODO: Aspect ratio */
		float rotationHorizontalDelta = -mouseDelta.x * PI / 3;
		float rotationVerticalDelta = mouseDelta.y * PI / 4;

		rotationHorizontal += rotationHorizontalDelta;
		rotationVertical = bound(-PI / 2, rotationVertical + rotationVerticalDelta, PI / 2 - VIEW_DELTA);
	}

	float3 lookDirection = float3(std::sin(rotationHorizontal), std::sin(rotationVertical), std::cos(rotationHorizontal));
	camera->setLookAt(position + lookDirection);
}
