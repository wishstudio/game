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

	void registerCube(u16 id, const std::string &tilePath);
	void registerCube(u16 id, Color color);
	bool isLightTransparent(u16 type) const;
	AABB3D getBoundingBox(const Block &block) const;
	bool isSolid(const Block &block) const;
	void drawBlock(TriangleCollector *collector, const Block &block) const;

	PTexture getTexture(u16 id) const;

private:
	struct BlockTypeSpec
	{
		Type type;
		Tile tile;
	} blockTypeSpec[65536];
};
