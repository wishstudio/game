#include "stdafx.h"

#include "Block.h"
#include "BlockType.h"
#include "Chunk.h"
#include "World.h"

Block::Block(Chunk *chunk, int in_x, int in_y, int in_z)
{
	this->chunk = chunk;
	this->in_x = in_x;
	this->in_y = in_y;
	this->in_z = in_z;
	this->data = &chunk->blocks[in_x][in_y][in_z];
}

int Block::x() const
{
	return in_x + chunk->chunk_x * CHUNK_SIZE;
}

int Block::y() const
{
	return in_y + chunk->chunk_y * CHUNK_SIZE;
}

int Block::z() const
{
	return in_z + chunk->chunk_z * CHUNK_SIZE;
}

Block Block::getNeighbour(int deltaX, int deltaY, int deltaZ) const
{
	int X = in_x + deltaX;
	int Y = in_y + deltaY;
	int Z = in_z + deltaZ;
	if (isInChunk(X, Y, Z))
		return Block(chunk, X, Y, Z);
	else
		return world->getBlock(x() + deltaX, y() + deltaY, z() + deltaZ);
}

Block Block::getNeighbour(Direction direction) const
{
	return getNeighbour(dirX[direction], dirY[direction], dirZ[direction]);
}

Block Block::tryGetNeighbour(int deltaX, int deltaY, int deltaZ) const
{
	int X = in_x + deltaX;
	int Y = in_y + deltaY;
	int Z = in_z + deltaZ;
	if (isInChunk(X, Y, Z))
		return Block(chunk, X, Y, Z);
	else
		return world->tryGetBlock(x() + deltaX, y() + deltaY, z() + deltaZ);
}

Block Block::tryGetNeighbour(Direction direction) const
{
	return tryGetNeighbour(dirX[direction], dirY[direction], dirZ[direction]);
}

AABB Block::getBoundingBox() const
{
	return blockType->getBoundingBox(*this);
}

bool Block::isSolid() const
{
	return blockType->isSolid(*this);
}

u16 Block::getType() const
{
	return data->type;
}

void Block::setType(u16 type)
{
	std::lock_guard<std::mutex> lock(chunk->accessMutex);
	data->type = type;
	chunk->setDirty(in_x, in_y, in_z);
}

u8 Block::getParam1() const
{
	return data->param1;
}

u8 Block::getParam2() const
{
	return data->param2;
}

void Block::setParam1(u8 param1)
{
	std::lock_guard<std::mutex> lock(chunk->accessMutex);
	data->param1 = param1;
	chunk->setDirty(in_x, in_y, in_z);
}

void Block::setParam2(u8 param2)
{
	std::lock_guard<std::mutex> lock(chunk->accessMutex);
	data->param2 = param2;
	chunk->setDirty(in_x, in_y, in_z);
}

void Block::setParam(u8 param1, u8 param2)
{
	std::lock_guard<std::mutex> lock(chunk->accessMutex);
	data->param1 = param1;
	data->param2 = param2;
	chunk->setDirty(in_x, in_y, in_z);
}

u8 Block::getLight() const
{
	return data->sunlight;
}
