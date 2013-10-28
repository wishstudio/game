#pragma once

#include "Block.h"

class Chunk;
class World: public Thread
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

	u32 getLoadedChunkCount() const volatile { return loadedChunkCount; }
	void save();

	Block getBlock(int x, int y, int z);
	Block tryGetBlock(int x, int y, int z);
	Chunk *getChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *tryGetChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *getChunkForBlock(int x, int y, int z);
	Chunk *tryGetChunkForBlock(int x, int y, int z);

	Chunk *preloadChunk(int chunk_x, int chunk_y, int chunk_z);
	void preloadChunkLight(Chunk *chunk);
	void preloadChunkBuffer(Chunk *chunk);
	void ensureChunkBufferLoaded(Chunk *chunk);
	void ensureChunkDataLoaded(Chunk *chunk);

	bool getCameraIntersection(const line3df &ray, CameraIntersectionInfo **info);

protected:
	virtual void run() override;

private:
	Concurrency::concurrent_queue<Chunk *> loadQueue;
	Concurrency::concurrent_unordered_map<std::tuple<int, int, int>, Chunk *> chunks;
	std::mutex chunksHashMutex;
	std::atomic<u32> loadedChunkCount;
};
