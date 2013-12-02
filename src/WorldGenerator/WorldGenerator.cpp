#include <stdafx.h>

#include "WorldGenerator.h"

#include "TerrainGenerator.h"

static std::once_flag onceflag;
static std::vector<std::unique_ptr<WorldGenerator>> generators;

static std::vector<std::unique_ptr<WorldGenerator>> &getGenerators()
{
	std::call_once(onceflag, [&]() {
		generators.push_back(std::unique_ptr<WorldGenerator>(new TerrainGenerator()));
	});
	return generators;
}

int WorldGenerator::getPhaseCount()
{
	return getGenerators().size();
}

WorldGenerator *WorldGenerator::getGenerator(int phase)
{
	return getGenerators()[phase].get();
}
