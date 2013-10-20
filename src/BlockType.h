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

	void registerCube(u16 id, stringw tilePath);
	bool isCube(u16 type) const;
	bool isLightTransparent(u16 type) const;
	aabbox3df getBoundingBox(const Block &block) const;
	void drawBlock(
		TriangleCollector *collector,
		const Block &block,
		bool xCovered, bool mxCovered, bool yCovered, bool myCovered, bool zCovered, bool mzCovered,
		u8 xLight, u8 mxLight, u8 yLight, u8 myLight, u8 zLight, u8 mzLight
	) const;

	ITexture* getTexture(u16 id) const;

private:
	struct BlockTypeSpec
	{
		Type type;
		Tile tile;
	} blockTypeSpec[65536];
};
