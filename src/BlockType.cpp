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

bool BlockType::isSolid(const Block &block) const
{
	return blockTypeSpec[block.getType()].type == Type_Cube;
}

ITexture* BlockType::getTexture(u16 id) const {
	if (blockTypeSpec[id].type != 0)
		return blockTypeSpec[id].tile.texture;
	else
		return nullptr;
}

static inline SColor LIGHT_COLOR(u8 light)
{
	int c = 255 * (light + 1) / 16.0;
	return SColor(255, c, c, c);
}

void BlockType::drawBlock(TriangleCollector *collector, const Block &block) const
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

		Block x = block.getNeighbour(DIRECTION_X);
		if (!x.isSolid())
			collector->addQuad(tile, p4, p2, p8, p6, vector3df(1, 0, 0), LIGHT_COLOR(x.getLight()));

		Block mx = block.getNeighbour(DIRECTION_MX);
		if (!mx.isSolid())
			collector->addQuad(tile, p1, p3, p5, p7, vector3df(-1, 0, 0), LIGHT_COLOR(mx.getLight()));

		Block y = block.getNeighbour(DIRECTION_Y);
		if (!y.isSolid())
			collector->addQuad(tile, p1, p2, p3, p4, vector3df(0, 1, 0), LIGHT_COLOR(y.getLight()));

		Block my = block.getNeighbour(DIRECTION_MY);
		if (!my.isSolid())
			collector->addQuad(tile, p7, p8, p5, p6, vector3df(0, -1, 0), LIGHT_COLOR(my.getLight()));

		Block z = block.getNeighbour(DIRECTION_Z);
		if (!z.isSolid())
			collector->addQuad(tile, p2, p1, p6, p5, vector3df(0, 0, 1), LIGHT_COLOR(z.getLight()));

		Block mz = block.getNeighbour(DIRECTION_MZ);
		if (!mz.isSolid())
			collector->addQuad(tile, p3, p4, p7, p8, vector3df(0, 0, -1), LIGHT_COLOR(mz.getLight()));
	}
}
