#pragma once

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <irrlicht.h>

using irr::u8;
using irr::u16;
using irr::u32;
using irr::u64;
using irr::s8;
using irr::s16;
using irr::s32;
using irr::s64;
using irr::f32;
using namespace irr::core;
using namespace irr::gui;
using namespace irr::io;
using namespace irr::scene;
using namespace irr::video;

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

#include "Engine/Camera.h"
#include "Engine/Color.h"
#include "Engine/ITexture.h"
#include "Engine/IVideo.h"
#include "Engine/IWindowSystem.h"
#include "Engine/Matrix4.h"
#include "Engine/Vector3.h"

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
extern TimeManager *timeManager;
extern World *world;
extern EventReceiver *eventReceiver;

extern Camera *camera;
extern IVideo *video;
extern IWindowSystem *windowSystem;

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
