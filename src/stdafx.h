#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "Engine/Types.h"

#include "Engine/AABB.h"
#include "Engine/Matrix4.h"
#include "Engine/Ray3D.h"
#include "Engine/Triangle3D.h"
#include "Engine/Vector2D.h"
#include "Engine/Vector2DI.h"
#include "Engine/Vector3D.h"
#include "Engine/Vector3DI.h"

#include "Engine/Camera.h"
#include "Engine/Color.h"
#include "Engine/Device.h"
#include "Engine/GUI.h"
#include "Engine/IndexBuffer.h"
#include "Engine/InputValue.h"
#include "Engine/Material.h"
#include "Engine/Pass.h"
#include "Engine/Shader.h"
#include "Engine/Texture.h"
#include "Engine/VertexBuffer.h"
#include "Engine/VertexFormat.h"
#include "Engine/Video.h"

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
	f32 u, v;

	Vertex() = default;
	Vertex(Vector3D _position, Color _color, f32 _u, f32 _v):
		position(_position), color(_color), u(_u), v(_v)
	{}
};

extern PDevice device;
extern PVideo video;
extern PGUI gui;
extern PVertexFormat vertexFormat;
