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

void BlockType::registerCube(int id, const std::string &tilePath)
{
	blockTypeSpec[id].type = Type_Cube;
	blockTypeSpec[id].tile = tileManager->registerTile(tilePath);
}

void BlockType::registerCube(int id, Color color)
{
	blockTypeSpec[id].type = Type_Cube;
	blockTypeSpec[id].tile = tileManager->registerTile(video->createTexture(1, 1, &color));
}

bool BlockType::isLightTransparent(int type) const
{
	return blockTypeSpec[type].type == Type_Air;
}

AABB3D BlockType::getBoundingBox(const Block &block) const
{
	if (blockTypeSpec[block.getType()].type == Type_Cube)
		return AABB3D(0, 0, 0, 1, 1, 1);
	else
		return AABB3D(0, 0, 0, 0, 0, 0);
}

bool BlockType::isSolid(const Block &block) const
{
	return blockTypeSpec[block.getType()].type == Type_Cube;
}

PTexture BlockType::getTexture(int id) const {
	if (blockTypeSpec[id].type != 0)
		return blockTypeSpec[id].tile.texture;
	else
		return nullptr;
}

static inline Color LIGHT_COLOR(int light)
{
	int c = 255 * (light + 1) / 16.0;
	return Color(c, c, c, 255);
}

static inline int VERTEX_LIGHT(const Block &base, const Block &side1, const Block &side2, const Block &corner)
{
	if (side1.isSolid() && side2.isSolid())
		return (base.getLight() + side1.getLight() + side2.getLight()) / 4;
	return (base.getLight() + side1.getLight() + side2.getLight() + corner.getLight()) / 4;
}

static void drawFace(
	TriangleCollector *collector,
	const Tile &tile,
	const float3 &p00, const float3 &p01,
	const float3 &p10, const float3 &p11,
	const Block &b00, const Block &b01, const Block &b02,
	const Block &b10, const Block &b11, const Block &b12,
	const Block &b20, const Block &b21, const Block &b22,
	const float3 &normal)
{
	/*
	 *    *--*--*--*
	 *    |00|01|02|
	 *    *--+--+--*
	 *    |10|11|12|
	 *    *--+--+--*
	 *    |20|21|22|
	 *    *--*--*--*
	 */
	if (!b11.isSolid())
	{
		int a00 = VERTEX_LIGHT(b11, b10, b01, b00);
		int a01 = VERTEX_LIGHT(b11, b01, b12, b02);
		int a10 = VERTEX_LIGHT(b11, b21, b10, b20);
		int a11 = VERTEX_LIGHT(b11, b12, b21, b22);
		collector->addQuad(tile,
			p00, LIGHT_COLOR(a00),
			p01, LIGHT_COLOR(a01),
			p10, LIGHT_COLOR(a10),
			p11, LIGHT_COLOR(a11),
			normal);
	}
}

void BlockType::drawBlock(TriangleCollector *collector, const Block &block) const
{
	collector->setCurrentBlock(block);
	int type = block.getType();
	if (blockTypeSpec[type].type == Type_Cube)
	{
		const Tile &tile = blockTypeSpec[type].tile;
		/*        |y
		 *        |
		 *        1----------2
		 *       /|         /|
		 *      / |        / |
		 *     /  |       /  |
		 *    3---+------4   |
		 *    |   5------+---6------x
		 *    |  /       |  /
		 *    | /        | /
		 *    |/         |/
		 *    7----------8
		 *   /
		 *  /z
		 */
		float3 p1(0, 1, 0);
		float3 p2(1, 1, 0);
		float3 p3(0, 1, 1);
		float3 p4(1, 1, 1);

		float3 p5(0, 0, 0);
		float3 p6(1, 0, 0);
		float3 p7(0, 0, 1);
		float3 p8(1, 0, 1);

		Block b[3][3][3];
		for (int x = -1; x <= 1; x++)
			for (int y = -1; y <= 1; y++)
				for (int z = -1; z <= 1; z++)
					b[x + 1][y + 1][z + 1] = block.getNeighbour(x, y, z);

		drawFace(collector, tile, p4, p2, p8, p6,
			b[2][2][2], b[2][2][1], b[2][2][0],
			b[2][1][2], b[2][1][1], b[2][1][0],
			b[2][0][2], b[2][0][1], b[2][0][0],
			float3(1, 0, 0));

		drawFace(collector, tile, p1, p3, p5, p7,
			b[0][2][0], b[0][2][1], b[0][2][2],
			b[0][1][0], b[0][1][1], b[0][1][2],
			b[0][0][0], b[0][0][1], b[0][0][2],
			float3(-1, 0, 0));

		drawFace(collector, tile, p1, p2, p3, p4,
			b[0][2][0], b[1][2][0], b[2][2][0],
			b[0][2][1], b[1][2][1], b[2][2][1],
			b[0][2][2], b[1][2][2], b[2][2][2],
			float3(0, 1, 0));

		drawFace(collector, tile, p7, p8, p5, p6,
			b[0][0][2], b[1][0][2], b[2][0][2],
			b[0][0][1], b[1][0][1], b[2][0][1],
			b[0][0][0], b[1][0][0], b[2][0][0],
			float3(0, -1, 0));

		drawFace(collector, tile, p3, p4, p7, p8,
			b[0][2][2], b[1][2][2], b[2][2][2],
			b[0][1][2], b[1][1][2], b[2][1][2],
			b[0][0][2], b[1][0][2], b[2][0][2],
			float3(0, 0, 1));

		drawFace(collector, tile, p2, p1, p6, p5,
			b[2][2][0], b[1][2][0], b[0][2][0],
			b[2][1][0], b[1][1][0], b[0][1][0],
			b[2][0][0], b[1][0][0], b[0][0][0],
			float3(0, 0, -1));
	}
}
