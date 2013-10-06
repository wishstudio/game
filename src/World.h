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

	u32 getLoadedCounkCount() const;

	Block getBlock(int x, int y, int z);
	Chunk *getChunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk *getChunkForBlock(int x, int y, int z);
	bool getCameraIntersection(const line3df &ray, CameraIntersectionInfo **info);

private:
	Hash<int, int, int, Chunk *> chunks;
};
