#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "Database.h"
#include "Serialization.h"
#include "TriangleCollector.h"
#include "World.h"
#include "WorldGenerator/WorldGenerator.h"
#include "WorldGenerator/WorldManipulator.h"

#define reenter(c) \
	switch (state) \
	if (false) \
	{ \
		_terminate_coroutine: \
		break; \
	} \
	else case 0:

#define yield_impl(cond, n) \
	if (state = (n), !(cond)) \
		goto _terminate_coroutine; \
	else case (n):

#ifdef _MSC_VER
#define yield(cond) yield_impl(cond, __COUNTER__ + 1)
#else
#define yield(cond) yield_impl(cond, __LINE__)
#endif

class ChunkRawDataTask: public IAsyncTask
{
public:
	ChunkRawDataTask(Chunk *_chunk): chunk(_chunk) {}
	virtual ~ChunkRawDataTask() override {}

	virtual void runAsync() override
	{
		reenter(state)
		{
			yield(chunk->accessMutex.lock(this));
			chunk->_loadRawData();
			chunk->accessMutex.unlock();
			chunk->rawDataCondition.notifyAll();
			chunk->_commonCallback();
			delete this;
		}
	}

private:
	Chunk *chunk;
	std::atomic<int> state;
};

class ChunkGenerateTask: public IAsyncTask
{
public:
	ChunkGenerateTask(Chunk *_chunk, int _phase): chunk(_chunk), phase(_phase) {}
	virtual ~ChunkGenerateTask() override {}

	virtual void runAsync() override
	{
		WorldGenerator *generator;
		Chunk *c;
		reenter(state)
		{
			if (phase > 0)
			{
				/* Ensure surrounding chunks are all done [phase - 1] at least */
				generator = WorldGenerator::getGenerator(phase);
				spanMin = generator->getSpanMin();
				spanMax = generator->getSpanMax();
				if (phase > 1)
				{
					/* Ensure surrounding older chunks requiring this chunk to be at most [phase - 1] are all done */
					/*
					 * x' + spanMin <= x <= x' + spanMax
					 *
					 * x - spanMax <= x' <= x - spanMin
					 */
					WorldGenerator *generator = WorldGenerator::getGenerator(phase - 1);
					int3 oldSpanMin = -generator->getSpanMax();
					int3 oldSpanMax = -generator->getSpanMin();

					spanMin.x = std::min(spanMin.x, oldSpanMin.x);
					spanMin.y = std::min(spanMin.y, oldSpanMin.y);
					spanMin.z = std::min(spanMin.z, oldSpanMin.z);

					spanMax.x = std::max(spanMax.x, oldSpanMax.x);
					spanMax.y = std::max(spanMax.y, oldSpanMax.y);
					spanMax.z = std::max(spanMax.z, oldSpanMax.z);
				}
				for (x = spanMin.x; x <= spanMax.x; x++)
					for (y = spanMin.y; y <= spanMax.y; y++)
						for (z = spanMin.z; z <= spanMax.z; z++)
						{
							c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
							yield(c->_generateAsync(phase - 1, this));
						}
			}

			/* Lock related chunks for current phase */
			generator = WorldGenerator::getGenerator(phase);
			spanMin = generator->getSpanMin();
			spanMax = generator->getSpanMax();

			/* Lock chunks */
			for (x = spanMin.x; x <= spanMax.x; x++)
				for (y = spanMin.y; y <= spanMax.y; y++)
					for (z = spanMin.z; z <= spanMax.z; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						yield(c->accessMutex.lock(this));
					}

			chunk->_generate(phase);

			/* Unlock chunks */
			for (x = spanMin.x; x <= spanMax.x; x++)
				for (y = spanMin.y; y <= spanMax.y; y++)
					for (z = spanMin.z; z <= spanMax.z; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						c->accessMutex.unlock();
					}

			if (phase + 1 == WorldGenerator::getPhaseCount())
				chunk->dataCondition.notifyAll();
			else
				chunk->generateCondition[phase].notifyAll();
			chunk->_commonCallback();
			delete this;
		}
	}

private:
	Chunk *chunk;
	int phase;
	std::atomic<int> state;
	int3 spanMin, spanMax;
	int x, y, z;
};

class ChunkLightTask: public IAsyncTask
{
public:
	ChunkLightTask(Chunk *_chunk): chunk(_chunk) {}
	virtual ~ChunkLightTask() override {}

	virtual void runAsync() override
	{
		Chunk *c;
		bool atTop;
		reenter(state)
		{
			atTop = chunk->chunk_y > 5;
			if (!atTop)
			{
				c = world->rawGetChunk(chunk->chunk_x, chunk->chunk_y + 1, chunk->chunk_z);
				yield(c->_loadLightAsync(this));
			}

			/* Ensure surrounding chunks have data */
			for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
					for (z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						yield(c->_loadDataAsync(this));
					}

			/* Lock surrounding chunks */
			for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
					for (z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						yield(c->accessMutex.lock(this));
					}
			
			chunk->_loadLight();

			/* Unlock chunks */
			for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
					for (z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						c->accessMutex.unlock();
					}
			chunk->lightCondition.notifyAll();
			chunk->_commonCallback();
			delete this;
		}
	}

private:
	Chunk *chunk;
	std::atomic<int> state;
	int x, y, z;
};

class ChunkBufferTask: public IAsyncTask
{
public:
	ChunkBufferTask(Chunk *_chunk): chunk(_chunk) {}
	virtual ~ChunkBufferTask() override {}

	virtual void runAsync() override
	{
		Chunk *c;
		reenter(state)
		{
			/* Ensuring all surrounding chunks are lighted (including self) */
			for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
					for (z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						yield(c->_loadLightAsync(this));
					}

			/* Lock surrounding chunks */
			for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
					for (z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						yield(c->accessMutex.lock(this));
					}

			chunk->_loadBuffer();

			/* Unlock chunks */
			for (int x = -1; x <= 1; x++)
				for (int y = -1; y <= 1; y++)
					for (int z = -1; z <= 1; z++)
					{
						c = world->rawGetChunk(chunk->chunk_x + x, chunk->chunk_y + y, chunk->chunk_z + z);
						c->accessMutex.unlock();
					}

			delete this;
		}
	}

private:
	Chunk *chunk;
	std::atomic<int> state;
	int x, y, z;
};

Serializer &operator << (Serializer &serializer, const BlockData &data)
{
	serializer << data.type << data.param1 << data.param2;
	return serializer;
}

Deserializer &operator >> (Deserializer &deserializer, BlockData &data)
{
	deserializer >> data.type >> data.param1 >> data.param2;
	return deserializer;
}

Chunk::Chunk(int _chunk_x, int _chunk_y, int _chunk_z)
	: chunk_x(_chunk_x), chunk_y(_chunk_y), chunk_z(_chunk_z), boundingBox({0, 0, 0})
{
	memset(blocks, 0, sizeof blocks);

	status = Status::Nothing;
	generationPhase = 0;
	triangleCollector = nullptr;

	dirty = false;

	boundingBox.merge(float3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE));
	modelTransform = float4x4::translation(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE);
}

Serializer &operator << (Serializer &serializer, const Chunk &data)
{
	if (data.getStatus() == Chunk::Status::Generating)
		serializer << data.generationPhase;
	else
		serializer << 0;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			for (int k = 0; k < CHUNK_SIZE; k++)
				serializer << data.blocks[i][j][k];
	return serializer;
}

Deserializer &operator >> (Deserializer &deserializer, Chunk &data)
{
	int generationPhase;
	deserializer >> generationPhase;
	if (generationPhase != 0)
	{
		data.status = Chunk::Status::Generating;
		data.generationPhase = generationPhase;
	}
	else
	{
		data.status = Chunk::Status::Data;
		data.generationPhase = WorldGenerator::getPhaseCount();
	}
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			for (int k = 0; k < CHUNK_SIZE; k++)
				deserializer >> data.blocks[i][j][k];
	return deserializer;
}

bool Chunk::_tryClaimWorking()
{
	return working.test_and_set() == false;
}

void Chunk::_raiseExpect(Status expectedStatus)
{
	_raiseExpect(expectedStatus, WorldGenerator::getPhaseCount());
}

void Chunk::_raiseExpect(Status _expectedStatus, int _expectedGenerationPhase)
{
	for (;;)
	{
		Status currentExpectedStatus = expectedStatus.load();
		int currentExpectedGenerationPhase = expectedGenerationPhase.load();
		if (currentExpectedStatus >= _expectedStatus && currentExpectedGenerationPhase >= _expectedGenerationPhase)
			break;
		if (currentExpectedGenerationPhase < _expectedGenerationPhase)
		{
			if (!expectedGenerationPhase.compare_exchange_weak(currentExpectedGenerationPhase, _expectedGenerationPhase))
				continue;
		}
		if (currentExpectedStatus < _expectedStatus)
			if (expectedStatus.compare_exchange_weak(currentExpectedStatus, _expectedStatus))
				break;
	}
}

void Chunk::_commonCallback()
{
	/* Must clear 'working' at first to avoid missing tasks */
	working.clear();
	if (status < expectedStatus || generationPhase < expectedGenerationPhase)
	{
		if (!_tryClaimWorking())
			/* tryClaimWorking() failed means another thread took over the task */
			return;
		/* Determine next thing to do */
		if (status == Status::Nothing)
			world->addTask(new ChunkRawDataTask(this));
		else if (status == Status::Generating)
			world->addTask(new ChunkGenerateTask(this, generationPhase));
		else if (status == Status::Data)
			world->addTask(new ChunkLightTask(this));
		else if (status == Status::Light)
			world->addTask(new ChunkBufferTask(this));
		else
			/* Some bad thing happened */;
	}
}

void Chunk::loadRawDataAsync()
{
	_loadRawDataAsync(nullptr);
}

bool Chunk::_loadRawDataAsync(IAsyncTask *callback)
{
	if (status > Status::Nothing)
		return true;

	/* These orderings are critical, don't change unless you're absolutely sure */

	_raiseExpect(Status::Generating, 0);

	if (_tryClaimWorking())
		_commonCallback();

	if (callback == nullptr)
		return true;

	if (rawDataCondition.tryAddWaitOn(callback))
		return true;
	return false;
}

void Chunk::_loadRawData()
{
	if (!database->loadChunk(this))
	{
		status = Status::Generating;
		generationPhase = 0;
	}
}

void Chunk::loadDataAsync()
{
	_loadDataAsync(nullptr);
}

bool Chunk::_loadDataAsync(IAsyncTask *callback)
{
	if (status >= Status::Data)
		return true;

	_raiseExpect(Status::Data);

	if (_tryClaimWorking())
		_commonCallback();

	if (callback == nullptr)
		return true;

	if (dataCondition.tryAddWaitOn(callback))
		return true;
	return false;
}

bool Chunk::_generateAsync(int phase, IAsyncTask *callback)
{
	if (generationPhase > phase)
		return true;
	if (phase + 1 == WorldGenerator::getPhaseCount())
		_raiseExpect(Status::Data);
	else
		_raiseExpect(Status::Generating, phase + 1);

	if (_tryClaimWorking())
		_commonCallback();

	if (phase + 1 == WorldGenerator::getPhaseCount())
	{
		if (dataCondition.tryAddWaitOn(callback))
			return true;
	}
	else
	{
		if (generateCondition[phase].tryAddWaitOn(callback))
			return true;
	}
	return false;
}

void Chunk::_generate(int phase)
{
	/* Add related chunks to WorldManipulator */
	WorldGenerator *generator = WorldGenerator::getGenerator(phase);
	int3 spanMin = generator->getSpanMin();
	int3 spanMax = generator->getSpanMax();
	WorldManipulator worldManipulator(spanMin.x, spanMin.y, spanMin.z, spanMax.x, spanMax.y, spanMax.z);
	for (int x = spanMin.x; x <= spanMax.x; x++)
		for (int y = spanMin.y; y <= spanMax.y; y++)
			for (int z = spanMin.z; z <= spanMax.z; z++)
			{
				Chunk *chunk = world->rawGetChunk(chunk_x + x, chunk_y + y, chunk_z + z);
				worldManipulator.addChunk(x, y, z, &chunk->blocks);
			}

	/* Generate current phase */
	/* TODO: Seeding */
	generator->generate(1, chunk_x, chunk_y, chunk_z, worldManipulator);
	generationPhase = phase + 1;

	if (phase + 1 == WorldGenerator::getPhaseCount())
		status = Status::Data;
}

void Chunk::loadLightAsync()
{
	_loadLightAsync(nullptr);
}

bool Chunk::_loadLightAsync(IAsyncTask *callback)
{
	if (status >= Status::Light)
		return true;

	_raiseExpect(Status::Light);

	if (_tryClaimWorking())
		_commonCallback();

	if (callback == nullptr)
		return true;

	if (lightCondition.tryAddWaitOn(callback))
		return true;
	return false;
}

int Chunk::_diminishLight(int light)
{
	return light - 2;
}

void Chunk::_unpropagateLight()
{
	Chunk *belowChunk = world->tryGetChunk(chunk_x, chunk_y - 1, chunk_z);
	while (!unpropagateQueue.empty())
	{
		Block block = unpropagateQueue.pop();
		int light = _diminishLight(block.data->sunlight);
		block.data->sunlight = 0;

		/* Invalidate neighbour buffers due to ambient occlusion in use */
		_invalidateMooreBuffer(block);
		if (light <= 0)
			continue;
		if (light == _diminishLight(15))
		{
			/* This is straight sunlight, unpropagate the block below */
			Block dest = block.tryGetNeighbour(DIRECTION_MY);
			if (dest.isValid() &&
				(dest.getChunk() == this || dest.getChunk()->getStatus() >= Status::Light) &&
				dest.data->sunlight == 15)
			{
				if (dest.getChunk() != this && dest.getChunk() != belowChunk)
				{
					/* Too far, just add it to its own unpropagate queue */
					dest.getChunk()->unpropagateQueue.push(dest);
					dest.getChunk()->_invalidateLight();
				}
				else
					unpropagateQueue.push(dest);
			}
		}
		for (int i = 0; i < DIRECTION_COUNT; i++)
		{
			Block dest = block.tryGetNeighbour((Direction)i);
			/* Ensure destination chunk has been lighted */
			if (dest.isValid() &&
				(dest.getChunk() == this || dest.getChunk()->getStatus() >= Status::Light) &&
				dest.data->sunlight <= light) /* So we can be sure that light is propagated from this block */
			{
				dest.getChunk()->_invalidateLight();
				unpropagateQueue.push(dest);
			}
		}
	}
}

void Chunk::_loadLight()
{
	/* Unpropagate removed lights if any exists */
	_unpropagateLight();

	/* TODO */
	bool atTop = chunk_y > 5;

	/* Propagate sunlight */
	if (atTop)
	{
		/* Above chunk not exist */
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blocks[x][CHUNK_SIZE - 1][z].sunlight =
					blockType->isLightTransparent(blocks[x][CHUNK_SIZE - 1][z].type)? 15: 0;
	}
	else
	{
		/* Propagate from above chunk */
		Chunk *chunk = world->getChunk(chunk_x, chunk_y + 1, chunk_z);
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				if (blockType->isLightTransparent(blocks[x][CHUNK_SIZE - 1][z].type)
					&& chunk->blocks[x][0][z].sunlight == 15)
					blocks[x][CHUNK_SIZE - 1][z].sunlight = 15;
				else
					blocks[x][CHUNK_SIZE - 1][z].sunlight = 0;
	}
	/* Propagate top -> down */
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = CHUNK_SIZE - 2; y >= 0; y--)
				if (blockType->isLightTransparent(blocks[x][y][z].type))
					blocks[x][y][z].sunlight = blocks[x][y + 1][z].sunlight;
				else
					blocks[x][y][z].sunlight = 0;

	/* Indirect illumination */
	Queue<Block> lightingQueue;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				if (_diminishLight(blocks[x][y][z].sunlight) > 0)
					lightingQueue.push(Block(this, x, y, z));
	/* Neighbour chunks */
	/* Order is consistent with Direction enum */
	int startX[DIRECTION_COUNT] = {              0,              0,              0, CHUNK_SIZE - 1,              0,              0};
	int   endX[DIRECTION_COUNT] = {              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	int startY[DIRECTION_COUNT] = {              0,              0,              0,              0, CHUNK_SIZE - 1,              0};
	int   endY[DIRECTION_COUNT] = { CHUNK_SIZE - 1,              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	int startZ[DIRECTION_COUNT] = {              0,              0,              0,              0,              0, CHUNK_SIZE - 1};
	int   endZ[DIRECTION_COUNT] = { CHUNK_SIZE - 1, CHUNK_SIZE - 1,              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	for (int i = 0; i < DIRECTION_COUNT; i++)
	{
		Chunk *chunk = world->tryGetChunk(chunk_x + dirX[i], chunk_y + dirY[i], chunk_z + dirZ[i]);
		if (chunk && chunk->getStatus() >= Status::Light)
		{
			for (int x = startX[i]; x <= endX[i]; x++)
				for (int y = startY[i]; y <= endY[i]; y++)
					for (int z = startZ[i]; z <= endZ[i]; z++)
						if (_diminishLight(chunk->blocks[x][y][z].sunlight) > 0)
							lightingQueue.push(Block(chunk, x, y, z));
		}
	}

	/* Calculate illumination */
	while (!lightingQueue.empty())
	{
		Block block = lightingQueue.pop();
		int light = _diminishLight(block.data->sunlight);
		for (int i = 0; i < DIRECTION_COUNT; i++)
		{
			Block dest = block.tryGetNeighbour((Direction) i);
			/* Ensure destination chunk has been lighted */
			if (dest.isValid() &&
				(dest.getChunk() == this || dest.getChunk()->getStatus() >= Status::Light) &&
				blockType->isLightTransparent(dest.getType()) &&
				light > dest.data->sunlight)
			{
				dest.data->sunlight = light;
				if (_diminishLight(light) > 0)
					lightingQueue.push(dest);
			}
		}
	}

	status = Status::Light;
}

void Chunk::loadBufferAsync()
{
	_loadBufferAsync();
}

void Chunk::_loadBufferAsync()
{
	if (status >= Status::Buffer)
		return;

	_raiseExpect(Status::Buffer);

	if (_tryClaimWorking())
		_commonCallback();
}

void Chunk::_loadBuffer()
{
	TriangleCollector *collector = new TriangleCollector();
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blockType->drawBlock(collector, Block(this, x, y, z));
	collector->finalize();
	
	TriangleCollector *oldCollector = triangleCollector.exchange(collector);
	if (oldCollector)
		world->asyncDeleteTriangleCollector(oldCollector);

	status = Status::Buffer;
}

/* Gets all triangles which have or may have contact within a specific bounding box */
void Chunk::getTriangles(std::vector<Triangle3D> &triangles, const AABB3D &box, const float4x4 &transform)
{
	if (status < Status::Buffer)
	{
		loadBufferAsync();
		while (status < Status::Buffer)
			/* TODO: Spin lock for now */;
	}

	TriangleCollector *collector = triangleCollector.load();

	float4x4 mat = modelTransform.getInversed();
	AABB3D tBox = box.transform(mat);

	mat = modelTransform * transform;

	int x_min = bound<int>(0, floor(tBox.minPoint.x), CHUNK_SIZE - 1);
	int y_min = bound<int>(0, floor(tBox.minPoint.y), CHUNK_SIZE - 1);
	int z_min = bound<int>(0, floor(tBox.minPoint.z), CHUNK_SIZE - 1);
	int x_max = bound<int>(0, ceil(tBox.maxPoint.x), CHUNK_SIZE - 1);
	int y_max = bound<int>(0, ceil(tBox.maxPoint.y), CHUNK_SIZE - 1);
	int z_max = bound<int>(0, ceil(tBox.maxPoint.z), CHUNK_SIZE - 1);
	for (int x = x_min; x <= x_max; x++)
		for (int y = y_min; y <= y_max; y++)
			for (int z = z_min; z <= z_max; z++)
			{
				for (auto it = collector->blockBegin(x, y, z); it != collector->blockEnd(x, y, z); it++)
				{
					triangles.push_back(Triangle3D(it->pointA * mat, it->pointB * mat, it->pointC * mat));
				}
			}
}

/*void *Chunk::operator new(size_t size)
{
	return world->getChunkMemoryPool()->allocateObject();
}

void Chunk::operator delete(void *chunk)
{
	world->getChunkMemoryPool()->deleteObject((Chunk *) chunk);
}*/

void Chunk::setDirty(int x, int y, int z)
{
	if (!dirty)
		world->asyncSaveChunk(this);
	dirty = true;
	_invalidateLight();
	if (blocks[x][y][z].sunlight != 0 && !blockType->isLightTransparent(blocks[x][y][z].type))
		unpropagateQueue.push(Block(this, x, y, z));
	else /* TODO: Do not invalidate if light values does not change */
		_invalidateMooreBuffer(Block(this, x, y, z));
}

void Chunk::_invalidateLight()
{
	status = Status::Data;
//	TODO
//	world->asyncLoadChunk(this);
}

void Chunk::_invalidateBuffer()
{
	if (status == Status::Buffer)
	{
		status = Status::Light;
//		TODO
//		world->asyncLoadChunk(this);
	}
}

void Chunk::_invalidateMooreBuffer(const Block &block)
{
	/* TODO: Optimization */
	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{
				Block dest = block.tryGetNeighbour(x, y, z);
				if (dest.isValid())
					dest.getChunk()->_invalidateBuffer();
			}
}

bool Chunk::shouldPreloadBuffer()
{
	return isInViewRange();
}

bool Chunk::isInViewRange()
{
	float3 position = camera->getPosition();
	int x = (int)floor(position.x / CHUNK_SIZE);
	int y = (int)floor(position.y / CHUNK_SIZE);
	int z = (int)floor(position.z / CHUNK_SIZE);
	int dist = abs(chunk_x - x) + abs(chunk_y - y) + abs(chunk_z - z);

	return dist <= 10;
}

void Chunk::render()
{
	if (!isInViewRange())
		return;

	TriangleCollector *collector = triangleCollector.load();
	if (collector == nullptr)
	{
		loadBufferAsync();
		return;
	}
	camera->apply(modelTransform);
	collector->render();
}

void Chunk::save()
{
	if (!dirty || status < Status::Data)
		return;
	database->saveChunk(this);
	dirty = false;
}
