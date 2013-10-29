#include "stdafx.h"

#include "Block.h"
#include "Chunk.h"
#include "Database.h"
#include "World.h"

World::World()
{
	loadedChunkCount = 0;
	shouldStop = false;
	workerThreads.push_back(std::thread(&World::run, this));
}

World::~World()
{
	for (auto it : chunks)
		it.second->drop();
	shouldStop = true;
	{
		std::lock_guard<std::mutex> lock(workerMutex);
		workerCondition.notify_all();
	}
	for (std::thread &thread : workerThreads)
		thread.join();
}

void World::lock()
{
	worldMutex.lock();
}

void World::unlock()
{
	worldMutex.unlock();
}

void World::update()
{
	/* Manually remove hardware-mapped mesh buffers
	 * (Irrlicht does this in a badly way)
	 */
	IMeshBuffer *buffer;
	while (bufferDeleteQueue.try_pop(buffer))
	{
		driver->removeHardwareBuffer(buffer);
		buffer->drop();
	}
}

void World::save()
{
	database->beginTransaction();
	for (auto it : chunks)
		it.second->save();
	database->commitTransaction();
}

void World::asyncDeleteBuffer(IMeshBuffer *buffer)
{
	bufferDeleteQueue.push(buffer);
}

void World::asyncLoadChunk(Chunk *chunk)
{
	loadQueue.push(chunk);
	std::lock_guard<std::mutex> lock(workerMutex);
	workerCondition.notify_all();
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
	Chunk *chunk = preloadChunk(chunk_x, chunk_y, chunk_z);
	ensureChunkDataLoaded(chunk);
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
	/* If we use floats, the chunk coordinates for block (x, y, z) is simply
	   (floor(x / CHUNK_SIZE), floor(y / CHUNK_SIZE), floor(z / CHUNK_SIZE))
	   But as ints are used, we have to use such tricks to make it correct. */
	int chunk_x = (x < 0)? ((x - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (x / CHUNK_SIZE);
	int chunk_y = (y < 0)? ((y - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (y / CHUNK_SIZE);
	int chunk_z = (z < 0)? ((z - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (z / CHUNK_SIZE);
	return getChunk(chunk_x, chunk_y, chunk_z);
}

Chunk *World::tryGetChunkForBlock(int x, int y, int z)
{
	int chunk_x = (x < 0)? ((x - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (x / CHUNK_SIZE);
	int chunk_y = (y < 0)? ((y - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (y / CHUNK_SIZE);
	int chunk_z = (z < 0)? ((z - (CHUNK_SIZE - 1)) / CHUNK_SIZE): (z / CHUNK_SIZE);
	return tryGetChunk(chunk_x, chunk_y, chunk_z);
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

void World::preloadChunkLight(Chunk *chunk)
{
	if (chunk->getStatus() == Chunk::Status::DataLoaded)
	{
		chunk->setStatus(Chunk::Status::LightLoading);
		asyncLoadChunk(chunk);
	}
}

void World::preloadChunkBuffer(Chunk *chunk)
{
	Chunk::Status status = chunk->getStatus();
	if (status == Chunk::Status::DataLoaded)
	{
		chunk->setStatus(Chunk::Status::LightLoading);
		asyncLoadChunk(chunk);
	}
	else if (status == Chunk::Status::LightLoaded)
	{
		chunk->setStatus(Chunk::Status::BufferLoading);
		asyncLoadChunk(chunk);
	}
}

void World::ensureChunkDataLoaded(Chunk *chunk)
{
	for (;;)
		if (chunk->getStatus() >= Chunk::Status::DataLoaded)
			return;
}

void World::ensureChunkBufferLoaded(Chunk *chunk)
{
	for (;;)
	{
		Chunk::Status status = chunk->getStatus();
		if (status == Chunk::Status::FullLoaded)
			return;
		if (status == Chunk::Status::DataLoaded)
		{
			chunk->setStatus(Chunk::Status::LightLoading);
			asyncLoadChunk(chunk);
		}
		if (status == Chunk::Status::LightLoaded)
		{
			chunk->setStatus(Chunk::Status::BufferLoading);
			asyncLoadChunk(chunk);
		}
	}
}

void World::run()
{
	while (!shouldStop)
	{
		Chunk *chunk;
		if (loadQueue.try_pop(chunk))
		{
			if (chunk->getStatus() < Chunk::Status::DataLoaded)
			{
				std::lock_guard<std::mutex> lock(worldMutex);
				chunk->loadData();
				loadedChunkCount++;
				chunk->setStatus(Chunk::Status::LightLoading);
				chunk->loadLight();
				/* It refused to load light, push it to queue again */
				if (chunk->getStatus() < Chunk::Status::LightLoaded)
					loadQueue.push(chunk);
			}
			else if (chunk->getStatus() < Chunk::Status::LightLoaded)
			{
				std::lock_guard<std::mutex> lock(worldMutex);
				chunk->loadLight();
				/* It refused to load light, push it to queue again */
				if (chunk->getStatus() < Chunk::Status::LightLoaded)
					loadQueue.push(chunk);
			}
			else if (chunk->getStatus() < Chunk::Status::FullLoaded)
			{
				chunk->loadBuffer();
				/* It refused to load buffer, push it to queue again */
				if (chunk->getStatus() < Chunk::Status::FullLoaded)
					loadQueue.push(chunk);
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

bool World::getCameraIntersection(const line3df &ray, CameraIntersectionInfo **info)
{
	/* Implementation is based on
	   "A Fast Voxel Traversal Algorithm for Ray Tracing"
	   John Amanatides, Andrew Woo
	   http://www.cse.yorku.ca/~amana/research/grid.pdf
	   http://www.devmaster.net/articles/raytracing_series/A%20faster%20voxel%20traversal%20algorithm%20for%20ray%20tracing.pdf */

	/* The cell in which the ray starts (in chunk coordinate system). */
	int x = (int) floor(ray.start.X);
	int y = (int) floor(ray.start.Y);
	int z = (int) floor(ray.start.Z);

	/* Determine which way we go. */
	const vector3df rayvec = ray.getVector();
	const int stepX = sgn(rayvec.X);
	const int stepY = sgn(rayvec.Y);
	const int stepZ = sgn(rayvec.Z);

	/* Calculate cell boundaries. When the step (i.e. direction sign) is positive,
	   the next boundary is AFTER our current position, meaning that we have to add 1.
	   Otherwise, it is BEFORE our current position, in which case we add nothing. */
	const int cellBoundaryX = x + (stepX > 0 ? 1 : 0);
	const int cellBoundaryY = y + (stepY > 0 ? 1 : 0);
	const int cellBoundaryZ = z + (stepZ > 0 ? 1 : 0);

	/* Determine how far we can travel along the ray before we hit a voxel boundary. */
	const f32 inf = std::numeric_limits<f32>::infinity();
	f32 tMaxX = iszero(rayvec.X) ? inf : (cellBoundaryX - ray.start.X) / rayvec.X;
	f32 tMaxY = iszero(rayvec.Y) ? inf : (cellBoundaryY - ray.start.Y) / rayvec.Y;
	f32 tMaxZ = iszero(rayvec.Z) ? inf : (cellBoundaryZ - ray.start.Z) / rayvec.Z;
 
	/* Determine how far we must travel along the ray before we have crossed a gridcell. */
	const f32 tDeltaX = iszero(rayvec.X) ? inf : (stepX / rayvec.X);
	const f32 tDeltaY = iszero(rayvec.Y) ? inf : (stepY / rayvec.Y);
	const f32 tDeltaZ = iszero(rayvec.Z) ? inf : (stepZ / rayvec.Z);

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
		aabbox3df box = block.getBoundingBox();
		translateBox(box, x, y, z);
		if (rayIntersectsWithBox(ray, box))
		{
			*info = new CameraIntersectionInfo(block, direction);
			return true;
		}
    }
	return false;
}
