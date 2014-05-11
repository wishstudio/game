#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <Sare.h>

using namespace sare;

#include <atomic>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <condition_variable>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Concurrent/ConcurrentMemoryPool.h"
#include "Concurrent/ConcurrentStack.h"
#include "Hash.h"
#include "MathHelper.h"
#include "Queue.h"

class ChunkSceneNode;
class Database;
class BlockType;
class TileManager;
class World;
extern ChunkSceneNode *chunkSceneNode;
extern Database *database;
extern BlockType *blockType;
extern TileManager *tileManager;
extern World *world;

/* TODO */
class IAsyncTask
{
public:
	virtual ~IAsyncTask() {}

	virtual void runAsync() = 0;
};

extern Camera *camera;

struct Vertex
{
	float3 position;
	Color color;
	float u, v;

	Vertex() = default;
	Vertex(float3 _position, Color _color, float _u, float _v):
		position(_position), color(_color), u(_u), v(_v)
	{}
};

extern PDevice device;
extern PVideo video;
extern PPainter painter;
extern PVertexFormat vertexFormat;
