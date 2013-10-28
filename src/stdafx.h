#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <irrlicht.h>

#include <atomic>
#include <chrono>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <mutex>
#include <unordered_map>

using namespace irr;
using namespace core;
using namespace gui;
using namespace io;
using namespace scene;
using namespace video;

#include "Hash.h"
#include "Math.h"
#include "Queue.h"
#include "Thread.h"

class Database;
class BlockType;
class TileManager;
class World;
class EventReceiver;
extern ISceneManager *smgr;
extern IVideoDriver *driver;
extern ICameraSceneNode *camera;
extern Database *database;
extern BlockType *blockType;
extern TileManager *tileManager;
extern World *world;
extern EventReceiver *eventReceiver;

static const int CHUNK_SIZE = 16;
