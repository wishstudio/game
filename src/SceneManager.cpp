#include "stdafx.h"

#include "Chunk.h"
#include "SceneManager.h"
#include "World.h"

SceneManager::SceneManager()
{
	metaSelector = smgr->createMetaTriangleSelector();
	ISceneNodeAnimator *animator = smgr->createCollisionResponseAnimator(
		metaSelector, camera, vector3df(0.8f, 1.8f, 0.8f), vector3df(0.f, -0.01f, 0.f), vector3df(0.f, 1.2f, 0.f));
	camera->addAnimator(animator);
	animator->drop();
	update();
}

SceneManager::~SceneManager()
{
	metaSelector->drop();
}

void SceneManager::update()
{
	updateTriangleSelectors();
	updateChunks();
}

void SceneManager::updateTriangleSelectors()
{
	/* Update triangle selectors for chunks.
	   In general, we need 2x2x2 chunks for collision detection.
	   For axis x, if our x coordinate is less than 50% of current chunk,
	   we test for chunk [x - 1] and [x], otherwise we test [x] and [x + 1].
	   The same applies for axis y and z. */

	/* Flag indicating whether a required selector was already existed. */
	bool flag[2][2][2] = { { { false, false }, { false, false } }, { { false, false }, { false, false } } };

	/* Calculating base coordinates */
	vector3df position = camera->getPosition();
	f32 fx = position.X / CHUNK_SIZE;
	f32 fy = position.Y / CHUNK_SIZE;
	f32 fz = position.Z / CHUNK_SIZE;

	int basex = (int) floor(fx);
	int basey = (int) floor(fy);
	int basez = (int) floor(fz);

	basex = (fx - basex < .5f) ? basex - 1 : basex;
	basey = (fy - basey < .5f) ? basey - 1 : basey;
	basez = (fz - basez < .5f) ? basez - 1 : basez;

	/* Remove triangle selectors which are out of range. */
	for (auto it = chunkSelectors.begin(); it != chunkSelectors.end(); )
	{
		ITriangleSelector *selector = (*it);
		Chunk *chunk = (Chunk *) selector;
		int x = chunk->x() - basex, y = chunk->y() - basey, z = chunk->z() - basez;
		if (x < 0 || x > 1 || y < 0 || y > 1 || z < 0 || z > 1)
		{
			/* This one should be removed. */
			it = chunkSelectors.erase(it);
			metaSelector->removeTriangleSelector(selector);
		}
		else
		{
			/* This one should be kept. Mark it in the flags array. */
			flag[x][y][z] = true;
			it++;
		}
	}

	/* Add missing triangle selectors */
	for (int x = 0; x <= 1; x++)
		for (int y = 0; y <= 1; y++)
			for (int z = 0; z <= 1; z++)
				if (!flag[x][y][z])
				{
					Chunk *chunk = world->getChunk(x + basex, y + basey, z + basez);
					chunkSelectors.push_back(chunk);
					metaSelector->addTriangleSelector(chunk);
				}
}

void SceneManager::updateChunks()
{
	vector3df position = camera->getPosition();
	int chunk_x = (int) floor(position.X / CHUNK_SIZE);
	int chunk_y = (int) floor(position.Y / CHUNK_SIZE);
	int chunk_z = (int) floor(position.Z / CHUNK_SIZE);
	for (int x = chunk_x - 5; x <= chunk_x + 5; x++)
		for (int y = chunk_y - 5; y <= chunk_y + 5; y++)
			for (int z = chunk_z - 5; z <= chunk_z + 5; z++)
				world->preloadChunk(x, y, z, true);
}
