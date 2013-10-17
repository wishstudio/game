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

	volatile u32 getLoadedChunkCount() const { return loadedChunkCount; }
	void save();

	Chunk *preloadChunk(int chunk_x, int chunk_y, int chunk_z);
	void preloadChunkBuffer(Chunk *chunk);
	void ensureChunkBufferLoaded(Chunk *chunk);
	Block getBlock(int x, int y, int z);
	Chunk *tryGetChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *getChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *getChunkForBlock(int x, int y, int z);
	bool getCameraIntersection(const line3df &ray, CameraIntersectionInfo **info);

protected:
	virtual void run() override;

private:
	SingleSafeQueue<Chunk *> loadQueue;
	Hash<int, int, int, Chunk *> chunks;
	volatile u32 loadedChunkCount;
};
