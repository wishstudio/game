#pragma once

class SceneManager
{
public:
	SceneManager();
	virtual ~SceneManager();

	void update();

private:
	void updateTriangleSelectors();
	void updateChunks();

	IMetaTriangleSelector *metaSelector;
	list<ITriangleSelector *> chunkSelectors;
	vector3di lastPosition;
};
