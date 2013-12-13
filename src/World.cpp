#include "stdafx.h"

#include "Block.h"
#include "Chunk.h"
#include "ChunkSceneNode.h"
#include "Database.h"
#include "World.h"

World::World()
{
	shouldStop = false;
	u32 cnt = std::thread::hardware_concurrency();
	for (u32 i = 0; i < cnt; i++)
		workerThreads.push_back(std::thread(&World::run, this));
}

World::~World()
{
	for (auto it : chunks)
		delete it.second;
	shouldStop = true;
	{
		std::lock_guard<std::mutex> lock(workerMutex);
		workerCondition.notify_all();
	}
	for (std::thread &thread : workerThreads)
		thread.join();
}

void World::update()
{
	/* Manually remove hardware-mapped mesh buffers
	 * (Irrlicht does this in a badly way)
	 */
	TriangleCollector *collector;
	while (triangleCollectorDeleteQueue.try_pop(collector))
		delete collector;
}

void World::tick()
{
}

void World::save()
{
	database->beginTransaction();
	Chunk *chunk;
	while (saveQueue.try_pop(chunk))
		chunk->save();
	database->commitTransaction();
}

void World::asyncDeleteTriangleCollector(TriangleCollector *collector)
{
	triangleCollectorDeleteQueue.push(collector);
}

void World::asyncLoadChunk(Chunk *chunk)
{
	if (chunk->isInQueue())
		return;
	loadQueue.push(chunk);
	std::lock_guard<std::mutex> lock(workerMutex);
	workerCondition.notify_all();
}

void World::asyncSaveChunk(Chunk *chunk)
{
	saveQueue.push(chunk);
}

Block World::getBlock(int x, int y, int z)
{
	return Block(getChunkForBlock(x, y, z), mod(x, CHUNK_SIZE), mod(y, CHUNK_SIZE), mod(z, CHUNK_SIZE));
}

Block World::tryGetBlock(int x, int y, int z)
{
	Chunk *chunk = tryGetChunkForBlock(x, y, z);
	if (chunk == nullptr)
		return Block();
	else
		return Block(chunk, mod(x, CHUNK_SIZE), mod(y, CHUNK_SIZE), mod(z, CHUNK_SIZE));
}

Chunk *World::getChunk(int chunk_x, int chunk_y, int chunk_z)
{
	Chunk *chunk = rawGetChunk(chunk_x, chunk_y, chunk_z);
	if (chunk->getStatus() < Chunk::Status::Data)
		chunk->loadData();
	return chunk;
}

Chunk *World::tryGetChunk(int chunk_x, int chunk_y, int chunk_z)
{
	auto it = chunks.find(std::make_tuple(chunk_x, chunk_y, chunk_z));
	if (it != chunks.end())
		return it->second;
	return nullptr;
}

Chunk *World::getChunkForBlock(int x, int y, int z)
{
	return getChunk(divide(x, CHUNK_SIZE), divide(y, CHUNK_SIZE), divide(z, CHUNK_SIZE));
}

Chunk *World::tryGetChunkForBlock(int x, int y, int z)
{
	return tryGetChunk(divide(x, CHUNK_SIZE), divide(y, CHUNK_SIZE), divide(z, CHUNK_SIZE));
}

Chunk *World::preloadChunk(int chunk_x, int chunk_y, int chunk_z)
{
	Chunk *chunk = tryGetChunk(chunk_x, chunk_y, chunk_z);
	if (chunk) /* Already preloaded */
		return chunk;

	/* Lock chunks hash */
	std::lock_guard<std::mutex> lock(chunksHashMutex);
	/* Double check */
	chunk = tryGetChunk(chunk_x, chunk_y, chunk_z);
	if (chunk)
		return chunk;

	chunk = new Chunk(chunk_x, chunk_y, chunk_z);
	chunks.insert(std::make_pair(std::make_tuple(chunk_x, chunk_y, chunk_z), chunk));
	asyncLoadChunk(chunk);
	return chunk;
}

Chunk *World::rawGetChunk(int chunk_x, int chunk_y, int chunk_z)
{
	Chunk *chunk = tryGetChunk(chunk_x, chunk_y, chunk_z);
	if (chunk)
		return chunk;

	/* Lock chunks hash */
	std::lock_guard<std::mutex> lock(chunksHashMutex);
	/* Double check */
	chunk = tryGetChunk(chunk_x, chunk_y, chunk_z);
	if (chunk)
		return chunk;

	chunk = new Chunk(chunk_x, chunk_y, chunk_z);
	chunks.insert(std::make_pair(std::make_tuple(chunk_x, chunk_y, chunk_z), chunk));
	return chunk;
}

void World::run()
{
	while (!shouldStop)
	{
		Chunk *chunk;
		if (loadQueue.try_pop(chunk))
		{
			chunk->setInQueue(false);
			Chunk::Status status = chunk->getStatus();
			if (status < Chunk::Status::Data)
				chunk->loadData();
			if (status < Chunk::Status::Light)
				chunk->loadLight();
			if (!chunk->shouldPreloadBuffer())
				chunk->setStatus(Chunk::Status::Light);
			else
			{
				chunk->loadBuffer();
				chunk->setStatus(Chunk::Status::Buffer);
			}
			continue;
		}
		
		/* No remaining chunks to process, suspend */
		{
			std::unique_lock<std::mutex> lock(workerMutex);
			/* Double check */
			if (loadQueue.empty())
				workerCondition.wait(lock);
			/* We do not need to worry about spurious wakeups */
		}
	}
}

bool World::getCameraIntersection(const Ray3D &ray, CameraIntersectionInfo **info)
{
	/* Implementation is based on
	   "A Fast Voxel Traversal Algorithm for Ray Tracing"
	   John Amanatides, Andrew Woo
	   http://www.cse.yorku.ca/~amana/research/grid.pdf
	   http://www.devmaster.net/articles/raytracing_series/A%20faster%20voxel%20traversal%20algorithm%20for%20ray%20tracing.pdf */

	/* The cell in which the ray starts (in chunk coordinate system). */
	int x = (int) floor(ray.start.x);
	int y = (int) floor(ray.start.y);
	int z = (int) floor(ray.start.z);

	/* Determine which way we go. */
	const int stepX = sgn(ray.direction.x);
	const int stepY = sgn(ray.direction.y);
	const int stepZ = sgn(ray.direction.z);

	/* Calculate cell boundaries. When the step (i.e. direction sign) is positive,
	   the next boundary is AFTER our current position, meaning that we have to add 1.
	   Otherwise, it is BEFORE our current position, in which case we add nothing. */
	const int cellBoundaryX = x + (stepX > 0 ? 1 : 0);
	const int cellBoundaryY = y + (stepY > 0 ? 1 : 0);
	const int cellBoundaryZ = z + (stepZ > 0 ? 1 : 0);

	/* Determine how far we can travel along the ray before we hit a voxel boundary. */
	const f32 inf = std::numeric_limits<f32>::infinity();
	f32 tMaxX = isZero(ray.direction.x) ? inf : (cellBoundaryX - ray.start.x) / ray.direction.x;
	f32 tMaxY = isZero(ray.direction.y) ? inf : (cellBoundaryY - ray.start.y) / ray.direction.y;
	f32 tMaxZ = isZero(ray.direction.z) ? inf : (cellBoundaryZ - ray.start.z) / ray.direction.z;
 
	/* Determine how far we must travel along the ray before we have crossed a gridcell. */
	const f32 tDeltaX = isZero(ray.direction.x) ? inf : (stepX / ray.direction.x);
	const f32 tDeltaY = isZero(ray.direction.y) ? inf : (stepY / ray.direction.y);
	const f32 tDeltaZ = isZero(ray.direction.z) ? inf : (stepZ / ray.direction.z);

	/* For step == 0, this does not matter. */
	const Direction directionX = (stepX > 0) ? DIRECTION_X : DIRECTION_MX;
	const Direction directionY = (stepY > 0) ? DIRECTION_Y : DIRECTION_MY;
	const Direction directionZ = (stepZ > 0) ? DIRECTION_Z : DIRECTION_MZ;

	Direction direction;
	Block block = getBlock(x, y, z);
	/* For each step, determine which distance to the next voxel boundary is lowest (i.e.
	   which voxel boundary is nearest) and walk that way. */
	for (int i = 0; i < 20; i++) /* TODO */
    {
		/* First one will be skipped. */
		if (tMaxX < tMaxY && tMaxX < tMaxZ)
        {
            /* tMax.X is the lowest, an YZ cell boundary plane is nearest. */
			x += stepX;
            tMaxX += tDeltaX;
			direction = directionX;
			block = block.getNeighbour(stepX, 0, 0);
        }
        else if (tMaxY < tMaxZ)
        {
            /* tMax.Y is the lowest, an XZ cell boundary plane is nearest. */
            y += stepY;
            tMaxY += tDeltaY;
			direction = directionY;
			block = block.getNeighbour(0, stepY, 0);
        }
        else
        {
            /* tMax.Z is the lowest, an XY cell boundary plane is nearest. */
            z += stepZ;
            tMaxZ += tDeltaZ;
			direction = directionZ;
			block = block.getNeighbour(0, 0, stepZ);
        }
		/* Test current one */
		AABB3D box = block.getBoundingBox().translate(x, y, z);
		if (rayIntersectsWithBox(ray, box))
		{
			*info = new CameraIntersectionInfo(block, direction);
			return true;
		}
    }
	return false;
}
