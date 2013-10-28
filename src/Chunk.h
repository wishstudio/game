#pragma once

#include "TriangleCollector.h"

class Serializer;
class Deserializer;

struct BlockData
{
	u16 type;
	u8 param1, param2;
	u8 sunlight;

	friend Serializer &operator << (Serializer &serializer, const BlockData &data);
	friend Deserializer &operator >> (Deserializer &deserializer, BlockData &data);
};

class Chunk: public ISceneNode, public ITriangleSelector
{
public:
	enum class Status: int { DataLoading, DataLoaded, LightLoading, LightLoaded, BufferLoading, FullLoaded };
	/* DataLoading: Data is being loading from disk or generating (in the queue).
	 * DataLoaded: Data is loaded.
	 * LightLoading: Light is being generated (in the queue).
	 * LightLoaded: Light is loaded.
	 * BufferLoading: Vertex buffer is being generated (in the queue).
	 * FullLoaded: All things are loaded.
	 */
	Chunk(int chunk_x, int chunk_y, int chunk_z);
	virtual ~Chunk();

	static void initDatabase();

	Status getStatus() const volatile { return status; }
	void setStatus(Status _status) { status = _status; }

	int x() const { return chunk_x; }
	int y() const { return chunk_y; }
	int z() const { return chunk_z; }
	void setDirty(int x, int y, int z);

	void loadData();
	void loadLight();
	void loadBuffer();
	void save();
	
	friend Serializer &operator << (Serializer &serializer, const Chunk &data);
	friend Deserializer &operator >> (Deserializer &deserializer, Chunk &data);

	/* ISceneNode */
	virtual void OnRegisterSceneNode() override;
	virtual const aabbox3df &getBoundingBox() const override { return boundingBox; }
	virtual void render() override;
	
	/* ITriangleSelector */
	virtual ISceneNode *getSceneNodeForTriangle(u32 triangleIndex) const override { return (ISceneNode *) this; }
	virtual ITriangleSelector *getSelector(u32 index) override { return this; }
	virtual const ITriangleSelector *getSelector(u32 index) const override { return this; }
	virtual u32 getSelectorCount() const { return 1; }
	virtual s32 getTriangleCount() const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const matrix4 *transform = nullptr
	) const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const aabbox3df &box,
		const matrix4 *transform = nullptr
	) const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const line3df &line,
		const matrix4 *transform = nullptr
	) const override;

private:
	void generate();
	void invalidateLight();
	
	std::atomic<Status> status;
	int chunk_x, chunk_y, chunk_z;
	bool dirty;
	TriangleCollector collector;
	BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	SMaterial material;
	aabbox3df boundingBox;

	friend class Block;
};
