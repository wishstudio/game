#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <Engine.h>

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

#include "Hash.h"
#include "MathHelper.h"
#include "Queue.h"

class ChunkSceneNode;
class Database;
class BlockType;
class TileManager;
class TimeManager;
class World;
class EventReceiver;
extern ChunkSceneNode *chunkSceneNode;
extern Database *database;
extern BlockType *blockType;
extern TileManager *tileManager;
extern World *world;

extern Camera *camera;

struct Vertex
{
	Vector3D position;
	Color color;
	float u, v;

	Vertex() = default;
	Vertex(Vector3D _position, Color _color, float _u, float _v):
		position(_position), color(_color), u(_u), v(_v)
	{}
};

extern PDevice device;
extern PVideo video;
extern PGUI gui;
extern PVertexFormat vertexFormat;
