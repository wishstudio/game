#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "ChunkSceneNode.h"
#include "Database.h"
#include "EventReceiver.h"
#include "PlayerAnimator.h"
#include "ShortcutItemUI.h"
#include "TileManager.h"
#include "TimeManager.h"
#include "World.h"

#include "Engine/D3D11/D3D11Video.h"
#include "Engine/WindowSystem/Win32WindowSystem.h"

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "sqlite3.lib")
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
	/* Create video device */
	eventReceiver = new EventReceiver();

	Win32WindowSystem *w = new Win32WindowSystem();
	w->init(1024, 768);
	windowSystem = w;

	D3D11Video *v = new D3D11Video();
	v->init(w);
	video = v;

	/*
	irr::IrrlichtDevice *device = irr::createDevice(EDT_DIRECT3D9, dimension2d<u32>(1024, 768), 16, false, false, false, eventReceiver);
	if (!device)
		return 1;

	device->setResizable(true);
	
	ShortcutItemUI shortcutIUI;
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	IFileSystem *fs = device->getFileSystem();

	camera = smgr->addCameraSceneNode();
	camera->setNearValue(0.2);
	device->getCursorControl()->setVisible(false);*/
	camera = new Camera(video);
	camera->setNearValue(-0.2);

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
	PlayerAnimator *playerAnimator = new PlayerAnimator();
	chunkSceneNode = new ChunkSceneNode();
	
	bool lastLeftMouseDown = false, lastRightMouseDown = false;
	while (windowSystem->processMessage())
	{
		/* In case of window size changes */
		//auto renderTargetSize = driver->getCurrentRenderTargetSize();
		//camera->setAspectRatio((f32) renderTargetSize.Width / (f32) renderTargetSize.Height);

		//driver->beginScene(true, true, SColor(255, 127, 200, 251));
		video->beginDraw();
		video->clearScreen();

		timeManager->update();
		
		bool leftMousePressed = false, rightMousePressed = false;
		if (eventReceiver->isLeftButtonDown() && lastLeftMouseDown == false)
			leftMousePressed = true;
		lastLeftMouseDown = eventReceiver->isLeftButtonDown();
		if (eventReceiver->isRightButtonDown() && lastRightMouseDown == false)
			rightMousePressed = true;
		lastRightMouseDown = eventReceiver->isRightButtonDown();

		World::CameraIntersectionInfo *info = nullptr;
		/*if (world->getCameraIntersection(line3df(camera->getPosition(), camera->getLookAt()), &info))
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
			if (rightMousePressed)
				info->block.getNeighbour(oppositeDirection(info->direction)).setType(shortcutIUI.getCurrentItem());
		}*/
		while (timeManager->tick())
		{
			world->tick();
			playerAnimator->tick();
		}
		world->update();
		world->save();
			
		playerAnimator->update();
		video->setViewMatrix(camera->getViewMatrix());
		video->setProjectionMatrix(camera->getProjectionMatrix());
		chunkSceneNode->render();
		//smgr->drawAll();

		//eventReceiver->update();
		/*shortcutIUI.show();
		driver->draw2DLine(vector2d<s32>(driver->getScreenSize().Width / 2 - 10, driver->getScreenSize().Height / 2),
		vector2d<s32>(driver->getScreenSize().Width / 2 + 10, driver->getScreenSize().Height / 2), SColor(255, 255, 255, 255));
		driver->draw2DLine(vector2d<s32>(driver->getScreenSize().Width / 2, driver->getScreenSize().Height / 2 - 10),
		vector2d<s32>(driver->getScreenSize().Width / 2, driver->getScreenSize().Height / 2 + 10), SColor(255, 255, 255, 255));*/

		//int fps = driver->getFPS();
		int fps = 0;
		Vector3 position = camera->getPosition();
		stringw s;
		s += "POS";
		s += "(";
		s += position.x;
		s += ", ";
		s += position.y;
		s += ", ";
		s += position.z;
		s += ") ";
		if (info) {
			s += "PT";
			s += "(";
			s += info->block.x();
			s += ", ";
			s += info->block.y();
			s += ", ";
			s += info->block.z();
			s += ") ";
		}
		s += "FPS: ";
		s += fps;
		s += ", ";
		s += "Avg Frame Time: ";
		s += 1000.f / fps;
		s += ", ";
		s += world->getLoadedChunkCount();
		s += " chunks loaded";
		s += ", ";
		s += world->getLoadedChunkCount() * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
		s += " blocks loaded";
		windowSystem->setWindowTitle(s.c_str());
		
		video->endDraw();
		windowSystem->onNewFrame();
	}

	delete playerAnimator;
	delete world;
	delete timeManager;

	delete database;
	return 0;
}
