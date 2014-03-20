#pragma once

#include "TileManager.h"

class Block;
class TriangleCollector;
class BlockType
{
public:
	enum Type { Type_Air, Type_Cube };

	BlockType();
	virtual ~BlockType();

	void registerCube(int id, const std::string &tilePath);
	void registerCube(int id, Color color);
	bool isLightTransparent(int type) const;
	AABB3D getBoundingBox(const Block &block) const;
	bool isSolid(const Block &block) const;
	void drawBlock(TriangleCollector *collector, const Block &block) const;

	PTexture getTexture(int id) const;

private:
	struct BlockTypeSpec
	{
		Type type;
		Tile tile;
	} blockTypeSpec[65536];
};
