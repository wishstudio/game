#pragma once

#include <sqlite3.h>
#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace gui;
using namespace io;
using namespace scene;
using namespace video;

#include "Hash.h"
#include "Math.h"

class World;
extern ISceneManager *smgr;
extern IVideoDriver *driver;
extern ICameraSceneNode *camera;
extern sqlite3 *db;
extern World *world;
