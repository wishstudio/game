#pragma once

static const int PRELOAD_DISTANCE = 5;

class ChunkSceneNode
{
public:
	ChunkSceneNode();

	void render();

private:
	Chunk *preloadChunk[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	Chunk *backup[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	int lastX = 0, lastY = 0, lastZ = 0;
//	SViewFrustum viewFrustum;
};
