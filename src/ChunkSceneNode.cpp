#include "stdafx.h"

#include "Chunk.h"
#include "ChunkSceneNode.h"
#include "World.h"

ChunkSceneNode::ChunkSceneNode()
{
	memset(preloadChunk, 0, sizeof preloadChunk);
}

void ChunkSceneNode::render()
{
	float3 position = camera->getPosition();
	int current_x = (int)floor(position.x / CHUNK_SIZE);
	int current_y = (int)floor(position.y / CHUNK_SIZE);
	int current_z = (int)floor(position.z / CHUNK_SIZE);

	memcpy(backup, preloadChunk, sizeof preloadChunk);
	memset(preloadChunk, 0, sizeof preloadChunk);

	for (int x = -PRELOAD_DISTANCE; x <= PRELOAD_DISTANCE; x++)
		for (int y = -PRELOAD_DISTANCE; y <= PRELOAD_DISTANCE; y++)
			for (int z = -PRELOAD_DISTANCE; z <= PRELOAD_DISTANCE; z++)
			{
				int dx = lastX + x - current_x;
				int dy = lastY + y - current_y;
				int dz = lastZ + z - current_z;
				if (dx >= -PRELOAD_DISTANCE && dx <= PRELOAD_DISTANCE && dy >= -PRELOAD_DISTANCE && dy <= PRELOAD_DISTANCE && dz >= -PRELOAD_DISTANCE && dz <= PRELOAD_DISTANCE)
					preloadChunk[dx + PRELOAD_DISTANCE][dy + PRELOAD_DISTANCE][dz + PRELOAD_DISTANCE] = backup[x + PRELOAD_DISTANCE][y + PRELOAD_DISTANCE][z + PRELOAD_DISTANCE];
			}
	//viewFrustum = *camera->getViewFrustum();
	for (int x = -PRELOAD_DISTANCE; x <= PRELOAD_DISTANCE; x++)
		for (int y = -PRELOAD_DISTANCE; y <= PRELOAD_DISTANCE; y++)
			for (int z = -PRELOAD_DISTANCE; z <= PRELOAD_DISTANCE; z++)
			{
				Chunk *p = preloadChunk[x + PRELOAD_DISTANCE][y + PRELOAD_DISTANCE][z + PRELOAD_DISTANCE];
				if (p == nullptr)
					p = preloadChunk[x + PRELOAD_DISTANCE][y + PRELOAD_DISTANCE][z + PRELOAD_DISTANCE] = world->preloadChunk(current_x + x, current_y + y, current_z + z);

				/* View frustum culling */
				/*AABB3Dox3df box(p->x() * CHUNK_SIZE, p->y() * CHUNK_SIZE, p->z() * CHUNK_SIZE,
					(p->x() + 1) * CHUNK_SIZE, (p->y() + 1) * CHUNK_SIZE, (p->z() + 1) * CHUNK_SIZE);
				bool visible = true;
				for (u32 i = 0; i < SViewFrustum::VF_PLANE_COUNT; i++)
				{
					auto relation = box.classifyPlaneRelation(viewFrustum.planes[i]);
					if (relation == ISREL3D_FRONT)
					{
						visible = false;
						break;
					}
				}
				if (visible)*/
					p->render();
			}
	lastX = current_x;
	lastY = current_y;
	lastZ = current_z;
}
