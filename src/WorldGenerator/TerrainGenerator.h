#pragma once

#include "WorldGenerator.h"

#include "WorldManipulator.h"

class TerrainGenerator final: public WorldGenerator
{
public:
	TerrainGenerator() {}
	virtual ~TerrainGenerator() {}

	virtual Vector3DI getSpanMin() const override { return { 0, 0, 0 }; }
	virtual Vector3DI getSpanMax() const override { return { 0, 0, 0 }; }

	virtual void generate(int seed, int chunk_x, int chunk_y, int chunk_z, const WorldManipulator &w) const override
	{
		Noise2D heightmap = getHeightMap();
		heightmap.generate(chunk_x * CHUNK_SIZE, chunk_z * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);

		Noise3D noise(1, 4, 1, 0.4);
		noise.setSpread(5, 5, 5);
		noise.generate(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int y = 0; y < CHUNK_SIZE; y++)
				for (int z = 0; z < CHUNK_SIZE; z++)
				{
					int yy = chunk_y * CHUNK_SIZE + y;
					int h = heightmap.getNoise(x, z);
					if (yy < h)
					{
						float density = noise.getNoise(x, y, z);
						w[x][y][z].type = density > -0.4;
					}
				}
	}
};