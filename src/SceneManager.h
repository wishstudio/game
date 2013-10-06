#pragma once

class ChunkTriangleSelector;
class SceneManager
{
public:
	SceneManager();
	virtual ~SceneManager();

	void update();

private:
	IMetaTriangleSelector *metaSelector;
	list<ChunkTriangleSelector *> chunkSelectors;
};
