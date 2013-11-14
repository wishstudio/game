#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "Engine/Types.h"

#include "Engine/AABB.h"
#include "Engine/Camera.h"
#include "Engine/Color.h"
#include "Engine/IIndexBuffer.h"
#include "Engine/ITexture.h"
#include "Engine/IVertexBuffer.h"
#include "Engine/IVertexFormat.h"
#include "Engine/IVideo.h"
#include "Engine/IWindowSystem.h"
#include "Engine/Matrix4.h"
#include "Engine/Ray3D.h"
#include "Engine/Triangle3D.h"
#include "Engine/Vector2D.h"
#include "Engine/Vector3D.h"

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
extern IVideo *video;
extern IWindowSystem *windowSystem;

struct Vertex
{
	Vector3D position;
	Color color;
	f32 u, v;
};

extern IVertexFormat *vertexFormat;
