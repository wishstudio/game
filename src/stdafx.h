#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace gui;
using namespace io;
using namespace scene;
using namespace video;

#include <atomic>
#include <chrono>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <condition_variable>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Hash.h"
#include "Math.h"
#include "Queue.h"

class Database;
class BlockType;
class TileManager;
class TimeManager;
class World;
class EventReceiver;
extern ISceneManager *smgr;
extern IVideoDriver *driver;
extern ICameraSceneNode *camera;
extern Database *database;
extern BlockType *blockType;
extern TileManager *tileManager;
extern TimeManager *timeManager;
extern World *world;
extern EventReceiver *eventReceiver;

/* 20 ticks per second */
static const auto TICK_DURATION = std::chrono::milliseconds(50);

/* Time helpers */
template <typename DestRep, typename SrcRep, typename SrcPeriod>
inline DestRep milliseconds(std::chrono::duration<SrcRep, SrcPeriod> duration)
{
	return std::chrono::duration_cast<std::chrono::duration<DestRep, std::ratio<1, 1000>>>(duration).count();
}

template <typename DestRep, typename SrcRep, typename SrcPeriod>
inline DestRep seconds(std::chrono::duration<SrcRep, SrcPeriod> duration)
{
	return std::chrono::duration_cast<std::chrono::duration<DestRep>>(duration).count();
}
