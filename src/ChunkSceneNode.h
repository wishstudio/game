#pragma once

static const int PRELOAD_DISTANCE = 5;

class ChunkSceneNode
{
public:
	ChunkSceneNode();

	const aabbox3df &getBoundingBox() const;
	void render();

private:
	Chunk *preloadChunk[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	Chunk *backup[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	vector3di lastPosition;
	aabbox3df box;
	SViewFrustum viewFrustum;
};
