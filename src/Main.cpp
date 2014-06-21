#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "ChunkSceneNode.h"
#include "Database.h"
#include "PlayerAnimator.h"
#include "ShortcutItemUI.h"
#include "TileManager.h"
#include "World.h"

#pragma comment(lib, "sqlite3.lib")
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")

static const char SHADER_SRC[] = R"DELIM(
cbuffer MatrixBuffer
{
	float4x4 mvpMatrix;
}

struct PS_InputType
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
	float2 tex: TEXCOORD;
};

PS_InputType VS_Main(float4 pos: POSITION, float4 color: COLOR, float2 tex: TEXCOORD)
{
	PS_InputType output;
	output.pos = mul(pos, mvpMatrix);
	output.color = color;
	output.tex = tex;
	return output;
}

SamplerState samplerState;
Texture2D shaderTexture;

float4 PS_Main(PS_InputType input): SV_TARGET
{
	return shaderTexture.Sample(samplerState, input.tex) * input.color;
}
)DELIM";

static PTexture whiteTexture = nullptr;

int main()
{
	/* Create video device */
	video = Video::createVideo(1366, 768);
	device = video->getDevice();
	painter.reset(new Painter(video));

	device->setMouseVisible(false);
	device->setTicksPerSecond(20);

	PVertexShader vertexShader = video->createVertexShader(SHADER_SRC, "VS_Main");
	PPixelShader pixelShader = video->createPixelShader(SHADER_SRC, "PS_Main");
	PMaterial defaultMaterial = video->createMaterial();
	{
		PPass pass = defaultMaterial->createPass();
		pass->setVertexShader(vertexShader);
		pass->setPixelShader(pixelShader);
	}

	PMaterial boxMaterial = video->createMaterial();
	{
		PPass pass = boxMaterial->createPass();
		pass->setVertexShader(vertexShader);
		pass->setPixelShader(pixelShader);
		pass->setAntialiasedLineEnabled(true);
	}

	Color whiteData = 0xFFFFFFFF;
	whiteTexture = video->createTexture(1, 1, &whiteData);
	PGeometryMesh gridMesh(new GeometryMesh(video));

	/* Create vertex format */
	vertexFormat = video->createVertexFormat();
	vertexFormat->addElement(TYPE_FLOAT3, "POSITION");
	vertexFormat->addElement(TYPE_UBYTE4_NORM, "COLOR");
	vertexFormat->addElement(TYPE_FLOAT2, "TEXCOORD");

	ShortcutItemUI shortcutItemUI;

	camera = new Camera(video);
	camera->setNearValue(-0.2);

	/* Initialize database */
	database = new Database();

	/* Initialize game logic */
	world = new World();
	tileManager = new TileManager();
	blockType = new BlockType();
	blockType->registerCube(1, Color(200, 200, 70, 255));
	blockType->registerCube(2, Color(0, 255, 255, 255));
	blockType->registerCube(3, Color(0, 255, 0, 255));
	PlayerAnimator *playerAnimator = new PlayerAnimator();
	chunkSceneNode = new ChunkSceneNode();
	
	bool vsync = false;
	bool uiOpened = false;
	while (device->beginFrame())
	{
		Event event;
		while (device->pollEvent(&event))
			/* Do nothing */;
		/*if (!device->getIsActive())
		{
			std::this_thread::yield();
			continue;
		}*/

		if (device->isKeyPressed(KEY_ESC))
		{
			uiOpened = !uiOpened;
			device->setMouseVisible(uiOpened);
			device->setNormalizedMousePosition(0, 0);
		}
		if (!uiOpened)
		{
			shortcutItemUI.update();
			World::CameraIntersectionInfo *info = nullptr;
			if (world->getCameraIntersection(Ray3D(camera->getPosition(), camera->getLookAt() - camera->getPosition()), &info))
			{
				if (device->isMousePressed(MOUSE_BUTTON_LEFT))
					info->block.setType(0);
				if (device->isMousePressed(MOUSE_BUTTON_RIGHT))
					info->block.getNeighbour(oppositeDirection(info->direction)).setType(shortcutItemUI.getCurrentItem());
			}
			playerAnimator->update(false);
			device->setNormalizedMousePosition(0, 0);
		}
		else
			playerAnimator->update(true);
		while (device->tick())
		{
			world->tick();
			playerAnimator->tick(uiOpened);
		}
		world->update();
		world->save();

		video->beginDraw(Color(127, 200, 251, 255));

		video->setMaterial(defaultMaterial);
		chunkSceneNode->render();
		World::CameraIntersectionInfo *info = nullptr;
		if (world->getCameraIntersection(Ray3D(camera->getPosition(), camera->getLookAt() - camera->getPosition()), &info))
		{
			AABB3D box = info->block.getBoundingBox().translate(info->block.x(), info->block.y(), info->block.z());
			video->setMaterial(boxMaterial);
			video->setTexture(whiteTexture);
			camera->apply(float4x4::identity());
			gridMesh->clear();
			gridMesh->addCubeFrame(box, Color(255, 0, 0, 255));
			gridMesh->render();
		}

		painter->beginDraw();
		shortcutItemUI.render();
		int2 bbSize = video->getBackBufferSize();
		painter->drawLine(bbSize.x / 2 - 10, bbSize.y / 2, bbSize.x / 2 + 10, bbSize.y / 2, 1, Color(255, 255, 255, 255));
		painter->drawLine(bbSize.x / 2, bbSize.y / 2 - 10, bbSize.x / 2, bbSize.y / 2 + 10, 1, Color(255, 255, 255, 255));
		painter->endDraw();
		video->endDraw();

		float3 position = camera->getPosition();
		std::wstringstream s;
		s.setf(std::ios::fixed, std::ios::floatfield);
		s << "POS(" << position.x << "," << position.y << "," << position.z << ") ";
		if (info)
			s << "PICK(" << info->block.x() << "," << info->block.y() << "," << info->block.z() << ") ";
		s << "FPS: " << device->getFPS() << " ";
		s << "Frame Time: " << device->getAverageFrameTime() * 1000 << "ms ";
		s << "Chunks: " << world->getLoadedChunkCount() << " ";
		s << "Blocks: " << world->getLoadedChunkCount() * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE << " ";
		s << "Vertices: " << video->getVertexCount() / 1000 << "k ";
		if (vsync)
			s << "VSYNC ON [F1]";
		else
			s << "VSYNC OFF [F1]";
		device->setWindowTitle(s.str().c_str());

		if (device->isKeyPressed(KEY_F1))
			vsync = !vsync;
		if (vsync)
		{
			int frameTime = device->getElapsedFrameTimeMilliseconds();
			device->delay(16 - frameTime);
		}
	}

	delete playerAnimator;
	delete world;

	delete database;
	return 0;
}
