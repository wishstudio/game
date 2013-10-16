#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "EventReceiver.h"
#include "SceneManager.h"
#include "TileManager.h"
#include "World.h"
#include "ShortcutItemUI.h"

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
	
	ShortcutItemUI shortcutIUI;
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	IFileSystem *fs = device->getFileSystem();

	SKeyMap keyMap[5];
	keyMap[0].Action = EKA_MOVE_FORWARD;
	keyMap[0].KeyCode = KEY_KEY_W;
	keyMap[1].Action = EKA_MOVE_BACKWARD;
	keyMap[1].KeyCode = KEY_KEY_S;
	keyMap[2].Action = EKA_STRAFE_LEFT;
	keyMap[2].KeyCode = KEY_KEY_A;
	keyMap[3].Action = EKA_STRAFE_RIGHT;
	keyMap[3].KeyCode = KEY_KEY_D;
	keyMap[4].Action = EKA_JUMP_UP;
	keyMap[4].KeyCode = KEY_SPACE;

	camera = smgr->addCameraSceneNodeFPS(nullptr, 80.f, 0.005f, -1, keyMap, 5, true, 0.005f);
	camera->setPosition(vector3df(0, 20, 0));
	camera->setTarget(vector3df(0, 0, 0));
	device->getCursorControl()->setVisible(false);

	/* Initialize database */
	if (sqlite3_open("data", &db) != SQLITE_OK)
		return 1;
	Chunk::initDatabase();

	/* Initialize game logic */
	world = new World();
	SceneManager *sceneManager = new SceneManager();
	tileManager = new TileManager();
	blockType = new BlockType();
	blockType->registerCube(1, "stonepile.png");
	blockType->registerCube(2, "magickaland.png");
	blockType->registerCube(3, "dirtroad.png");

	u16 handItem = 1;
	
	bool lastLeftMouseDown = false, lastRightMouseDown = false;
	while (device->run())
	{
		if (device->isWindowActive())
		{
			driver->beginScene(true, true, SColor(255, 127, 200, 251));

			bool leftMousePressed = false, rightMousePressed = false;
			if (eventReceiver->isLeftButtonDown() && lastLeftMouseDown == false)
				leftMousePressed = true;
			lastLeftMouseDown = eventReceiver->isLeftButtonDown();
			if (eventReceiver->isRightButtonDown() && lastRightMouseDown == false)
				rightMousePressed = true;
			lastRightMouseDown = eventReceiver->isRightButtonDown();

			sceneManager->update();
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
					info->block.neighbour(oppositeDirection(info->direction)).setType(handItem);
				}
			}

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
			device->setWindowCaption(s.c_str());

			driver->endScene();
		}
		else
			device->yield();
	}

	delete sceneManager;
	delete world;

	sqlite3_close(db);
	device->drop();
	return 0;
}
