#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "Database.h"
#include "EventReceiver.h"
#include "PlayerAnimator.h"
#include "SceneManager.h"
#include "ShortcutItemUI.h"
#include "TileManager.h"
#include "TimeManager.h"
#include "World.h"

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "sqlite3.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
	/* Create video device */
	eventReceiver = new EventReceiver();

	IrrlichtDevice *device = createDevice(EDT_DIRECT3D9, dimension2d<u32>(1024, 768), 16, false, false, false, eventReceiver);
	if (!device)
		return 1;

	device->setResizable(true);
	
	ShortcutItemUI shortcutIUI;
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	IFileSystem *fs = device->getFileSystem();

	camera = smgr->addCameraSceneNodeFPS(nullptr, 100.f, 0.f, -1, nullptr, 0);
	camera->setNearValue(0.2);
	device->getCursorControl()->setVisible(false);

	/* Initialize database */
	database = new Database();

	/* Initialize game logic */
	timeManager = new TimeManager();
	world = new World();
	tileManager = new TileManager();
	blockType = new BlockType();
	blockType->registerCube(1, "stonepile.png");
	blockType->registerCube(2, "magickaland.png");
	blockType->registerCube(3, "dirtroad.png");
	SceneManager *sceneManager = new SceneManager();
	PlayerAnimator *playerAnimator = new PlayerAnimator();

	u16 handItem = 1;
	
	bool lastLeftMouseDown = false, lastRightMouseDown = false;
	while (device->run())
	{
		if (device->isWindowActive())
		{
			driver->beginScene(true, true, SColor(255, 127, 200, 251));

			timeManager->update();

			bool leftMousePressed = false, rightMousePressed = false;
			if (eventReceiver->isLeftButtonDown() && lastLeftMouseDown == false)
				leftMousePressed = true;
			lastLeftMouseDown = eventReceiver->isLeftButtonDown();
			if (eventReceiver->isRightButtonDown() && lastRightMouseDown == false)
				rightMousePressed = true;
			lastRightMouseDown = eventReceiver->isRightButtonDown();

			sceneManager->update();
			world->lock();
			World::CameraIntersectionInfo *info;
			if (world->getCameraIntersection(line3df(camera->getPosition(), camera->getTarget()), &info))
			{
				driver->setTransform(ETS_WORLD, IdentityMatrix);
				aabbox3df box = info->block.getBoundingBox();
				translateBox(box, info->block.x(), info->block.y(), info->block.z());
				box.MinEdge.X -= .01f;
				box.MinEdge.Y -= .01f;
				box.MinEdge.Z -= .01f;
				box.MaxEdge.X += .01f;
				box.MaxEdge.Y += .01f;
				box.MaxEdge.Z += .01f;
				driver->draw3DBox(box, SColor(255, 255, 0, 0));
				if (leftMousePressed)
					info->block.setType(0);
				if (rightMousePressed) {
					handItem = shortcutIUI.getCurrentItem();
					info->block.getNeighbour(oppositeDirection(info->direction)).setType(handItem);
				}
			}
			world->unlock();

			while (timeManager->tick())
			{
				world->lock();
				world->tick();
				world->unlock();

				playerAnimator->tick();
				eventReceiver->tick();
			}
			world->update();
			world->save();
			
			playerAnimator->update();
			smgr->drawAll();

			shortcutIUI.show();
			driver->draw2DLine(vector2d<s32>(driver->getScreenSize().Width / 2 - 10, driver->getScreenSize().Height / 2),
			vector2d<s32>(driver->getScreenSize().Width / 2 + 10, driver->getScreenSize().Height / 2), SColor(255, 255, 255, 255));
			driver->draw2DLine(vector2d<s32>(driver->getScreenSize().Width / 2, driver->getScreenSize().Height / 2 - 10),
			vector2d<s32>(driver->getScreenSize().Width / 2, driver->getScreenSize().Height / 2 + 10), SColor(255, 255, 255, 255));

			int fps = driver->getFPS();
			vector3df position = camera->getPosition();
			stringw s;
			s = s + "(";
			s += position.X;
			s += ", ";
			s += position.Y;
			s += ", ";
			s += position.Z;
			s += ") ";
			s += "FPS: ";
			s += fps;
			s += ", ";
			s += world->getLoadedChunkCount();
			s += " chunks loaded";
			s += ", ";
			s += world->getLoadedChunkCount() * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
			s += " blocks loaded";
			device->setWindowCaption(s.c_str());

			driver->endScene();
		}
		else
			device->yield();
	}

	device->drop(); /* Make sure references are released first */
	delete playerAnimator;
	delete sceneManager;
	delete world;
	delete timeManager;

	delete database;
	return 0;
}
