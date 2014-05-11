#pragma once

class Serializer;
class Deserializer;
class TriangleCollector;

#include "Block.h"
#include "World.h"

class SpinLock final
{
public:
	void lock()
	{
		while (flag.test_and_set())
			/* Wait */;
	}

	void unlock()
	{
		flag.clear();
	}

private:
	std::atomic_flag flag;
};

class AsyncCondition final
{
public:
	AsyncCondition() { signalFlag = false; }

	bool tryAddWaitOn(IAsyncTask *callback)
	{
		if (signalFlag)
			return true;
		std::lock_guard<SpinLock> lock(accessLock);
		if (signalFlag)
			return true;
		waitingTasks.push(callback);
		return false;
	}

	void notifyAll()
	{
		std::lock_guard<SpinLock> lock(accessLock);
		signalFlag = true;
		IAsyncTask *task;
		while (waitingTasks.try_pop(task))
			world->addTask(task);
	}

private:
	SpinLock accessLock;
	std::atomic<bool> signalFlag; /* false -> Not signaled, true -> Signaled */
	Concurrency::concurrent_queue<IAsyncTask *> waitingTasks;
};

class AsyncMutex final
{
public:
	bool lock(IAsyncTask *callback)
	{
		if (flag.test_and_set() == true)
		{
			waitingTasks.push(callback);
			if (flag.test_and_set() == false)
			{
				IAsyncTask *task;
				if (waitingTasks.try_pop(task))
					world->addTask(task);
				else
					unlock();
			}
			return false;
		}
		else
			return true;
	}

	void unlock()
	{
		flag.clear();
		IAsyncTask *task;
		if (waitingTasks.try_pop(task))
		{
			if (lock(task))
				world->addTask(task);
		}
	}

private:
	Concurrency::concurrent_queue<IAsyncTask *> waitingTasks;
	std::atomic_flag flag;
};

struct BlockData final
{
	uint16_t type;
	uint8_t param1, param2;
	uint8_t sunlight;

	friend Serializer &operator << (Serializer &serializer, const BlockData &data);
	friend Deserializer &operator >> (Deserializer &deserializer, BlockData &data);
};

class Chunk final
{
public:
	enum class Status : int { Nothing, Generating, Data, Light, Buffer };
	/* Represents what has been loaded so far */
	Chunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk(const Chunk &) = delete;

	Status getStatus() const volatile { return status; }
	void setStatus(Status _status) { status = _status; }

	int x() const { return chunk_x; }
	int y() const { return chunk_y; }
	int z() const { return chunk_z; }

	void loadRawDataAsync();
	void loadDataAsync();
	void loadLightAsync();
	void loadBufferAsync();

	void save();
	void render();

	void getTriangles(std::vector<Triangle3D> &triangles, const AABB3D &box, const float4x4 &transform);

	bool shouldPreloadBuffer();
	bool isInViewRange();
	void setDirty(int x, int y, int z);

	friend Serializer &operator << (Serializer &serializer, const Chunk &data);
	friend Deserializer &operator >> (Deserializer &deserializer, Chunk &data);

	//static void *operator new(size_t size);
	//static void operator delete(void *chunk);

private:
	bool _tryClaimWorking();
	void _raiseExpect(Status expectedStatus);
	void _raiseExpect(Status expectedStatus, int expectedGenerationPhase);
	void _commonCallback();

	bool _loadRawDataAsync(IAsyncTask *callback);
	void _loadRawData();

	bool _loadDataAsync(IAsyncTask *callback);
	bool _generateAsync(int phase, IAsyncTask *callback);
	void _generate(int phase);
	
	bool _loadLightAsync(IAsyncTask *callback);
	void _loadLight();

	void _loadBufferAsync();
	void _loadBuffer();

	void _invalidateLight();
	void _invalidateBuffer();
	void _invalidateMooreBuffer(const Block &block);
	int _diminishLight(int light);
	void _unpropagateLight();

	const int chunk_x, chunk_y, chunk_z;
	std::atomic<Status> status, expectedStatus;
	std::atomic<int> generationPhase, expectedGenerationPhase; /* Current phase to be done */

	/* TODO: Size of generationCondition[] */
	AsyncCondition rawDataCondition, generateCondition[5], dataCondition, lightCondition;
	AsyncMutex accessMutex;
	std::atomic<bool> dirty;
	std::atomic_flag working; /* Working at now or in queue */
	std::atomic<TriangleCollector *> triangleCollector;
	BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	AABB3D boundingBox;
	float4x4 modelTransform;
	Queue<Block> unpropagateQueue;

	friend class Block; /* TODO */
	friend class ChunkRawDataTask;
	friend class ChunkGenerateTask;
	friend class ChunkLightTask;
	friend class ChunkBufferTask;
};
