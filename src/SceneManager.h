#pragma once

class SceneManager
{
public:
	SceneManager();
	virtual ~SceneManager();

	void update();

private:
	IMetaTriangleSelector *metaSelector;
	list<ITriangleSelector *> chunkSelectors;
};
