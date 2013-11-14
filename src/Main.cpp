#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "ChunkSceneNode.h"
#include "Database.h"
#include "PlayerAnimator.h"
#include "ShortcutItemUI.h"
#include "TileManager.h"
#include "World.h"

#include "Engine/D3D11/D3D11Video.h"
#include "Engine/WindowSystem/Win32WindowSystem.h"

#pragma comment(lib, "sqlite3.lib")
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")

int main()
{
	/* Create video device */
	Win32WindowSystem *w = new Win32WindowSystem();
	w->init(1024, 768);
	windowSystem = w;
	windowSystem->setMouseVisible(false);
	windowSystem->setTicksPerSecond(20);

	D3D11Video *v = new D3D11Video();
	v->init(w);
	video = v;

	/* Create vertex format */
	vertexFormat = video->createVertexFormat();
	vertexFormat->addElement(TYPE_FLOAT3, SEMANTIC_POSITION);
	vertexFormat->addElement(TYPE_UBYTE4_NORM, SEMANTIC_COLOR);
	vertexFormat->addElement(TYPE_FLOAT2, SEMANTIC_TEXCOORD);

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
	world = new World();
	tileManager = new TileManager();
	blockType = new BlockType();
	blockType->registerCube(1, "stonepile.png");
	blockType->registerCube(2, "magickaland.png");
	blockType->registerCube(3, "dirtroad.png");
	PlayerAnimator *playerAnimator = new PlayerAnimator();
	chunkSceneNode = new ChunkSceneNode();
	
	while (windowSystem->processMessage())
	{
		if (!windowSystem->isActive())
		{
			std::this_thread::yield();
			continue;
		}
		/* In case of window size changes */
		//auto renderTargetSize = driver->getCurrentRenderTargetSize();
		//camera->setAspectRatio((f32) renderTargetSize.Width / (f32) renderTargetSize.Height);

		//driver->beginScene(true, true, SColor(255, 127, 200, 251));
		video->beginDraw();
		video->clearScreen();
		
		World::CameraIntersectionInfo *info = nullptr;
		if (world->getCameraIntersection(Ray3D(camera->getPosition(), camera->getLookAt() - camera->getPosition()), &info))
		{
			AABB box = info->block.getBoundingBox();
			box.translate(info->block.x(), info->block.y(), info->block.z());
			/*box.MinEdge.X -= .01f;
			box.MinEdge.Y -= .01f;
			box.MinEdge.Z -= .01f;
			box.MaxEdge.X += .01f;
			box.MaxEdge.Y += .01f;
			box.MaxEdge.Z += .01f;
			driver->setTransform(ETS_WORLD, IdentityMatrix);
			driver->draw3DBox(box, SColor(255, 255, 0, 0));*/
			if (windowSystem->isMousePressed(MOUSE_BUTTON_LEFT))
				info->block.setType(0);
			if (windowSystem->isMousePressed(MOUSE_BUTTON_RIGHT))
				info->block.getNeighbour(oppositeDirection(info->direction)).setType(1);//shortcutIUI.getCurrentItem()
		}
		while (windowSystem->tick())
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

		Vector3D position = camera->getPosition();
		std::wstringstream s;
		s.setf(std::ios::fixed, std::ios::floatfield);
		s << "POS(" << position.x << "," << position.y << "," << position.z << ") ";
		if (info)
			s << "PICK(" << info->block.x() << "," << info->block.y() << "," << info->block.z() << ") ";
		s << "FPS: " << windowSystem->getFPS() << " ";
		s << "Frame Time: " << windowSystem->getAverageFrameTime() * 1000 << "ms ";
		s << "Chunks: " << world->getLoadedChunkCount() << " ";
		s << "Blocks: " << world->getLoadedChunkCount() * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE << " ";
		windowSystem->setWindowTitle(s.str().c_str());
		
		video->endDraw();
	}

	delete playerAnimator;
	delete world;

	delete database;
	return 0;
}
