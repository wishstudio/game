#include "stdafx.h"

#include "Chunk.h"
#include "SceneManager.h"
#include "World.h"

SceneManager::SceneManager()
{
	lastPosition = vector3di(INT_MAX / 2); /* Prevent overflow */
}

SceneManager::~SceneManager()
{
}

void SceneManager::update()
{
	vector3df position = camera->getPosition();
	int chunk_x = floor(position.X / CHUNK_SIZE);
	int chunk_y = floor(position.Y / CHUNK_SIZE);
	int chunk_z = floor(position.Z / CHUNK_SIZE);
	const int PRELOAD_DISTANCE = 5; /* TODO */
	for (int d = 0; d <= PRELOAD_DISTANCE; d++)
		for (int x = chunk_x - d; x <= chunk_x + d; x++)
			for (int y = chunk_y - d; y <= chunk_y + d; y++)
				for (int z = chunk_z - d; z <= chunk_z + d; z++)
					/* Reduce redundant calls */
					if (abs(x - lastPosition.X) > PRELOAD_DISTANCE ||
						abs(y - lastPosition.Y) > PRELOAD_DISTANCE ||
						abs(z - lastPosition.Z) > PRELOAD_DISTANCE)
						world->preloadChunk(x, y, z);
	lastPosition = { chunk_x, chunk_y, chunk_z };
}
