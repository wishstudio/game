#pragma once

class TreeGenerator final: public WorldGenerator
{
public:
	TreeGenerator() {}
	virtual ~TreeGenerator() {}

	virtual int3 getSpanMin() const override { return { -1, 0, -1 }; };
	virtual int3 getSpanMax() const override { return {  1, 1,  1 }; };

	virtual void generate(int seed, int chunk_x, int chunk_y, int chunk_z, const WorldManipulator &w) const override
	{
		Noise2D heightmap = getHeightMap(seed);
		heightmap.generate(chunk_x * CHUNK_SIZE, chunk_z * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);

		Noise2D treeProbability(seed, 3, 1, 0.8);
		treeProbability.setSpread(8, 8);
		treeProbability.generate((chunk_x - 1) * CHUNK_SIZE, (chunk_z - 1) * CHUNK_SIZE, CHUNK_SIZE * 3, CHUNK_SIZE * 3);
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				float m = -1;
				for (int xx = x - 5; xx <= x + 5; xx++)
					for (int zz = z - 5; zz <= z + 5; zz++)
						m = max(m, treeProbability.getNoise(xx + CHUNK_SIZE, zz + CHUNK_SIZE));
				if (m == treeProbability.getNoise(x + CHUNK_SIZE, z + CHUNK_SIZE) && m > 0)
				{
					int y = heightmap.getNoise(x, z) - chunk_y * CHUNK_SIZE;
					if (y >= 0 && y < CHUNK_SIZE && w[x][y][z].type != 0) /* TODO */
					{
						int trunkLen = 8;
						for (int yy = 1; yy <= trunkLen; yy++)
							w[x][y + yy][z].type = 2;
						for (int xx = x - 2; xx <= x + 2; xx++)
							for (int yy = y + trunkLen - 2; yy <= y + trunkLen + 2; yy++)
								for (int zz = z - 2; zz <= z + 2; zz++)
									w[xx][yy][zz].type = 3;
					}
				}
			}
	}
};
