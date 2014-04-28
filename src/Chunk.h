#pragma once

class Serializer;
class Deserializer;
class TriangleCollector;

#include "Block.h"
#include "World.h"

class AsyncCondition final
{
public:
	void waitOn(const AsyncTask &callback)
	{
		waitingTasks.push(callback);
	}

	void notifyAll()
	{
		AsyncTask task;
		while (waitingTasks.try_pop(task))
			world->addTask(task);
	}

private:
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
		AsyncTask task;
		if (waitingTasks.try_pop(task))
			world->addTask(task); /* Seamlessly hand over mutex */
		else
			flag.clear();
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

	static void *operator new(size_t size);
	static void operator delete(void *chunk);

private:
	bool _tryClaimWorking();
	void _raiseExpect(Status expectedStatus);
	void _raiseExpect(Status expectedStatus, int expectedGenerationPhase);
	void _commonCallback();

	void _loadRawDataAsync(const AsyncTask &callback);

	void _loadDataAsync(const AsyncTask &callback);
	void _generateAsync(int phase, const AsyncTask &callback);
	void _generateAsync_lock(int phase, const AsyncTask &callback);
	void _generate(int phase);
	
	void _loadLightAsync(const AsyncTask &callback);
	void _loadLightAsync_self(const AsyncTask &callback);
	void _loadLightAsync_lock(const AsyncTask &callback);
	void _loadLight();

	void _loadBufferAsync(const AsyncTask &callback);
	void _loadBufferAsync_lock(const AsyncTask &callback);
	void _loadBuffer();

	void _invalidateLight();
	void _invalidateBuffer();
	void _invalidateMooreBuffer(const Block &block);
	int _diminishLight(int light);
	void _unpropagateLight();

	const int chunk_x, chunk_y, chunk_z;
	std::atomic<Status> status, expectedStatus;
	std::atomic<int> generationPhase, expectedGenerationPhase; /* Current phase to be done */

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
