#pragma once

#include "TriangleCollector.h"

static const int CHUNK_SIZE = 16;

class Serializer;
class Deserializer;

struct BlockData
{
	u16 type;
	u8 param1, param2;

	friend Serializer &operator << (Serializer &serializer, const BlockData &data);
	friend Deserializer &operator >> (Deserializer &deserializer, BlockData &data);
};

class Chunk: public ISceneNode
{
public:
	Chunk(int chunk_x, int chunk_y, int chunk_z);
	virtual ~Chunk();

	static void initDatabase();

	int x() const { return chunk_x; }
	int y() const { return chunk_y; }
	int z() const { return chunk_z; }
	void setDirty(int x, int y, int z);

	/* ISceneNode */
	virtual void OnRegisterSceneNode() override;
	virtual const aabbox3df &getBoundingBox() const override { return boundingBox; }
	virtual void render() override;

private:
	void load();
	void save();
	void generate();
	void invalidateMeshBuffer();
	void createMeshBuffer();

	int chunk_x, chunk_y, chunk_z;
	bool dirty, triangleSelectorDirty, bufferDirty;
	TriangleCollector collector;
	BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	SMaterial material;
	aabbox3df boundingBox;

	friend class Block;
	friend class ChunkTriangleSelector;
};
