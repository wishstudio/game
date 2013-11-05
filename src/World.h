#pragma once

#include "Block.h"

class Chunk;
class World
{
public:
	struct CameraIntersectionInfo
	{
		Block block;
		Direction direction;
		
		CameraIntersectionInfo(Block _block, Direction _direction): block(_block), direction(_direction) {}
	};

	World();
	virtual ~World();

	void update();
	void tick();

	void asyncDeleteTriangleCollector(TriangleCollector *collector);

	u32 getLoadedChunkCount() const { return chunks.size(); }
	void save();
	void asyncLoadChunk(Chunk *chunk);

	Block getBlock(int x, int y, int z);
	Block tryGetBlock(int x, int y, int z);
	Chunk *getChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *tryGetChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *getChunkForBlock(int x, int y, int z);
	Chunk *tryGetChunkForBlock(int x, int y, int z);

	Chunk *preloadChunk(int chunk_x, int chunk_y, int chunk_z);

	bool getCameraIntersection(const line3df &ray, CameraIntersectionInfo **info);

private:
	void run();

	Concurrency::concurrent_queue<TriangleCollector *> triangleCollectorDeleteQueue;

	Concurrency::concurrent_unordered_map<std::tuple<int, int, int>, Chunk *> chunks;
	std::mutex chunksHashMutex;

	Concurrency::concurrent_queue<Chunk *> loadQueue;
	std::vector<std::thread> workerThreads;
	std::condition_variable workerCondition;
	std::mutex workerMutex;
	std::atomic<bool> shouldStop;
};
