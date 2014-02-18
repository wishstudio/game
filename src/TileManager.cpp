#include "stdafx.h"

#include "TileManager.h"

TileManager::TileManager()
{
}

TileManager::~TileManager()
{
}

Tile TileManager::registerTile(const std::string &resourcePath)
{
	return registerTile(video->createTexture("resources/" + resourcePath));
}

Tile TileManager::registerTile(PTexture texture)
{
	Tile tile;
	tile.texture = texture;
	tile.u1 = 0.f;
	tile.v1 = 0.f;
	tile.u2 = 1.f;
	tile.v2 = 1.f;
	return tile;
}
