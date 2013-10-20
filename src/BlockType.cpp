#include "stdafx.h"

#include "Block.h"
#include "BlockType.h"
#include "TileManager.h"
#include "TriangleCollector.h"

BlockType::BlockType()
{
	memset(blockTypeSpec, 0, sizeof blockTypeSpec);
}

BlockType::~BlockType()
{
}

void BlockType::registerCube(u16 id, stringw tilePath)
{
	blockTypeSpec[id].type = Type_Cube;
	blockTypeSpec[id].tile = tileManager->registerTile(tilePath);
}

bool BlockType::isCube(u16 type) const
{
	return blockTypeSpec[type].type == Type_Cube;
}

bool BlockType::isLightTransparent(u16 type) const
{
	return blockTypeSpec[type].type == Type_Air;
}

aabbox3df BlockType::getBoundingBox(const Block &block) const
{
	if (blockTypeSpec[block.getType()].type == Type_Cube)
		return aabbox3df(0, 0, 0, 1, 1, 1);
	else
		return aabbox3df(0, 0, 0, 0, 0, 0);
}

ITexture* BlockType::getTexture(u16 id) const {
	if (blockTypeSpec[id].type != 0)
		return blockTypeSpec[id].tile.texture;
	else
		return nullptr;
}

static inline SColor LIGHT_COLOR(u8 light)
{
	int c = 255 * light / 15.0;
	return SColor(255, c, c, c);
}

void BlockType::drawBlock(
	TriangleCollector *collector,
	const Block &block,
	bool xCovered, bool mxCovered, bool yCovered, bool myCovered, bool zCovered, bool mzCovered,
	u8 xLight, u8 mxLight, u8 yLight, u8 myLight, u8 zLight, u8 mzLight
	) const
{
	collector->setCurrentBlock(block);
	u16 type = block.getType();
	if (blockTypeSpec[type].type == Type_Cube)
	{
		const Tile &tile = blockTypeSpec[type].tile;
		/*
		 *        1          2
		 *        *----------*
		 *       /|         /|
		 *      / |        / |
		 *   3 /  |     4 /  |
		 *    *---+------*   |
		 *    |   *------+---*
		 *    |  / 5     |  / 6
		 *    | /        | /
		 *    |/         |/
		 *    *----------*
		 *     7          8
		 *
		 */
		vector3df p1(0, 1, 1);
		vector3df p2(1, 1, 1);
		vector3df p3(0, 1, 0);
		vector3df p4(1, 1, 0);

		vector3df p5(0, 0, 1);
		vector3df p6(1, 0, 1);
		vector3df p7(0, 0, 0);
		vector3df p8(1, 0, 0);

		if (!xCovered)
			collector->addQuad(tile, p4, p2, p8, p6, vector3df(1, 0, 0), LIGHT_COLOR(xLight));
		if (!mxCovered)
			collector->addQuad(tile, p1, p3, p5, p7, vector3df(-1, 0, 0), LIGHT_COLOR(mxLight));
		if (!yCovered)
			collector->addQuad(tile, p1, p2, p3, p4, vector3df(0, 1, 0), LIGHT_COLOR(yLight));
		if (!myCovered)
			collector->addQuad(tile, p7, p8, p5, p6, vector3df(0, -1, 0), LIGHT_COLOR(myLight));
		if (!zCovered)
			collector->addQuad(tile, p2, p1, p6, p5, vector3df(0, 0, 1), LIGHT_COLOR(zLight));
		if (!mzCovered)
			collector->addQuad(tile, p3, p4, p7, p8, vector3df(0, 0, -1), LIGHT_COLOR(mzLight));
	}
}
