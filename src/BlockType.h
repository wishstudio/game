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
	bool isLightTransparent(u16 type) const;
	AABB getBoundingBox(const Block &block) const;
	bool isSolid(const Block &block) const;
	void drawBlock(TriangleCollector *collector, const Block &block) const;

	ITTexture* getTexture(u16 id) const;

private:
	struct BlockTypeSpec
	{
		Type type;
		Tile tile;
	} blockTypeSpec[65536];
};
