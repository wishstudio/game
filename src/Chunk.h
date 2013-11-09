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

class Chunk
{
public:
	enum class Status : int { Nothing, Data, Light, Buffer };
	/* Represents what has been loaded so far */
	Chunk(int chunk_x, int chunk_y, int chunk_z);
	virtual ~Chunk();

	static void initDatabase();

	Status getStatus() const volatile { return status; }
	void setStatus(Status _status) { status = _status; }
	bool isInQueue() const volatile { return inQueue; }
	void setInQueue(bool _inQueue) { inQueue = _inQueue; }

	bool shouldPreloadBuffer();
	bool isInViewRange();

	int x() const { return chunk_x; }
	int y() const { return chunk_y; }
	int z() const { return chunk_z; }
	void setDirty(int x, int y, int z);

	void loadData();
	void loadLight();
	void loadBuffer();
	void save();
	void render();

	void getTriangles(std::vector<triangle3df> &triangles, const AABB &box, const Matrix4 &transform);

	friend Serializer &operator << (Serializer &serializer, const Chunk &data);
	friend Deserializer &operator >> (Deserializer &deserializer, Chunk &data);

private:
	void generate();
	void invalidateLight();

	const int chunk_x, chunk_y, chunk_z;
	std::mutex accessMutex;
	std::atomic<Status> status;
	std::atomic<bool> inQueue;
	std::atomic<bool> dirty;
	std::atomic<TriangleCollector *> triangleCollector;
	BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	AABB boundingBox;
	Matrix4 modelTransform;

	friend class Block;
};
