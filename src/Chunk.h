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

	bool tryAddWaitOn(const AsyncTask &callback)
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
		AsyncTask task;
		while (waitingTasks.try_pop(task))
			world->addTask(task);
	}

private:
	SpinLock accessLock;
	std::atomic<bool> signalFlag; /* false -> Not signaled, true -> Signaled */
	Concurrency::concurrent_queue<AsyncTask> waitingTasks;
};

class AsyncMutex final
{
public:
	void lock(const AsyncTask &callback)
	{
		if (flag.test_and_set() == true)
			waitingTasks.push(callback);
		else
			callback();
	}

	void unlock()
	{
		flag.clear();
		AsyncTask task;
		if (waitingTasks.try_pop(task))
			lock(task);
	}

private:
	Concurrency::concurrent_queue<AsyncTask> waitingTasks;
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

	void _loadRawDataAsync(const AsyncTask &callback);
	void _loadRawDataAsync_work();

	void _loadDataAsync(const AsyncTask &callback);
	void _generateAsync(int phase, const AsyncTask &callback);
	void _generateAsync_work(int phase);
	void _generateAsync_lock(int phase);
	void _generate(int phase);
	
	void _loadLightAsync(const AsyncTask &callback);
	void _loadLightAsync_work();
	void _loadLightAsync_lock();
	void _loadLight();

	void _loadBufferAsync();
	void _loadBufferAsync_work();
	void _loadBufferAsync_lock();
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

	friend class Block;
};
