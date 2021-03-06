#pragma once

struct BlockData;
class Chunk;
class Block
{
public:
	Block(): chunk(nullptr), in_x(0), in_y(0), in_z(0), data(nullptr) {}
	Block(Chunk *chunk, int in_x, int in_y, int in_z);
	Block(const Block &block): chunk(block.chunk), in_x(block.in_x), in_y(block.in_y), in_z(block.in_z), data(block.data) {}

	inline Block &operator = (const Block &block)
	{
		chunk = block.chunk;
		in_x = block.in_x;
		in_y = block.in_y;
		in_z = block.in_z;
		data = block.data;
		return *this;
	}

	inline bool isValid() const { return chunk != nullptr; }
	inline void invalidate()
	{
		chunk = nullptr;
		in_x = in_y = in_z = 0;
		data = nullptr;
	}

	int x() const;
	int y() const;
	int z() const;
	inline int innerX() const { return in_x; }
	inline int innerY() const { return in_y; }
	inline int innerZ() const { return in_z; }
	Block getNeighbour(int deltaX, int deltaY, int deltaZ) const;
	Block getNeighbour(Direction direction) const;
	Block tryGetNeighbour(int deltaX, int deltaY, int deltaZ) const;
	Block tryGetNeighbour(Direction direction) const;

	Chunk *getChunk() const { return chunk; }
	AABB3D getBoundingBox() const;
	bool isSolid() const;

	int getType() const;
	void setType(int type);
	int getParam1() const;
	int getParam2() const;
	void setParam1(int param1);
	void setParam2(int param2);
	void setParam(int param1, int param2);
	int getLight() const;

private:
	Chunk *chunk;
	int in_x, in_y, in_z;
	BlockData *data;

	friend class Chunk;
};
