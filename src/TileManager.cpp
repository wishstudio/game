#include "stdafx.h"

#include "TileManager.h"

TileManager::TileManager()
{
}

TileManager::~TileManager()
{
}

Tile TileManager::registerTile(stringw resourcePath)
{
	Tile tile;
	tile.texture = driver->getTexture(stringw("resources/") + resourcePath);
	tile.u1 = 0.f;
	tile.v1 = 0.f;
	tile.u2 = 1.f;
	tile.v2 = 1.f;
	return tile;
}
