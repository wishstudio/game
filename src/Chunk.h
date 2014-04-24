#pragma once

class Serializer;
class Deserializer;
class TriangleCollector;

#include "Block.h"

struct BlockData final
{
	uint16_t type;
	uint8_t param1, param2;
	uint8_t sunlight;

	friend Serializer &operator << (Serializer &serializer, const BlockData &data);
	friend Deserializer &operator >> (Deserializer &deserializer, BlockData &data);
};

class Chunk final
{
public:
	enum class Status : int { Nothing, Generating, Data, Light, Buffer };
	/* Represents what has been loaded so far */
	Chunk(int chunk_x, int chunk_y, int chunk_z);
	Chunk(const Chunk &) = delete;

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

	void getTriangles(std::vector<Triangle3D> &triangles, const AABB3D &box, const float4x4 &transform);

	friend Serializer &operator << (Serializer &serializer, const Chunk &data);
	friend Deserializer &operator >> (Deserializer &deserializer, Chunk &data);

	static void *operator new(size_t size);
	static void operator delete(void *chunk);

private:
	void _generate(int phase);
	void _loadRawData();
	void _invalidateLight();
	void _invalidateBuffer();
	void _invalidateMooreBuffer(const Block &block);
	int _diminishLight(int light);
	void _unpropagateLight();

	const int chunk_x, chunk_y, chunk_z;
	std::mutex accessMutex;
	std::atomic<Status> status;
	std::atomic<int> generationPhase; /* Current phase to be done */
	std::atomic<bool> inQueue;
	std::atomic<bool> dirty;
	std::atomic<TriangleCollector *> triangleCollector;
	BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	AABB3D boundingBox;
	float4x4 modelTransform;
	Queue<Block> unpropagateQueue;

	friend class Block;
};
