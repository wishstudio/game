#pragma once

static const int PRELOAD_DISTANCE = 5;

class ChunkSceneNode: public ISceneNode
{
public:
	ChunkSceneNode();

	virtual void OnRegisterSceneNode() override;
	virtual const aabbox3df &getBoundingBox() const override;
	virtual void render() override;

private:
	Chunk *preloadChunk[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	Chunk *backup[PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1][PRELOAD_DISTANCE * 2 + 1];
	vector3di lastPosition;
	aabbox3df box;
	SViewFrustum viewFrustum;
};
