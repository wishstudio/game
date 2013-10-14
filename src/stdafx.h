#pragma once

#include <sqlite3.h>
#include <irrlicht.h>
#include <unordered_map>

using namespace irr;
using namespace core;
using namespace gui;
using namespace io;
using namespace scene;
using namespace video;

#include "Hash.h"
#include "Math.h"

class BlockType;
class World;
class TileManager;
class EventReceiver;
extern ISceneManager *smgr;
extern IVideoDriver *driver;
extern ICameraSceneNode *camera;
extern sqlite3 *db;
extern BlockType *blockType;
extern TileManager *tileManager;
extern World *world;
extern EventReceiver *eventReceiver;

static const int CHUNK_SIZE = 16;
