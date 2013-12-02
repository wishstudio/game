#pragma once

#include <Noise.h>

/* Interface for world generators */
class WorldManipulator;
class WorldGenerator
{
public:
	WorldGenerator() {}
	virtual ~WorldGenerator() {}

	virtual Vector3DI getSpanMin() const = 0;
	virtual Vector3DI getSpanMax() const = 0;
	virtual void generate(int seed, int chunk_x, int chunk_y, int chunk_z, const WorldManipulator &w) const = 0;

	Noise2D getHeightMap(int seed) const;

	static int getPhaseCount();
	static WorldGenerator *getGenerator(int phase);
};
