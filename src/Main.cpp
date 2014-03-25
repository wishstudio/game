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

PS_InputType VS_Main(float4 pos: SV_POSITION, float4 color: COLOR, float2 tex: TEXCOORD)
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

static void draw3DBox(const AABB3D &box, Color color)
{
	static PVertexBuffer buffer = video->createVertexBuffer(vertexFormat, 36);

	video->setTexture(whiteTexture);
	camera->apply(float4x4::identity());

	float3 p000(box.minPoint.x, box.minPoint.y, box.minPoint.z);
	float3 p001(box.minPoint.x, box.minPoint.y, box.maxPoint.z);
	float3 p010(box.minPoint.x, box.maxPoint.y, box.minPoint.z);
	float3 p011(box.minPoint.x, box.maxPoint.y, box.maxPoint.z);
	float3 p100(box.maxPoint.x, box.minPoint.y, box.minPoint.z);
	float3 p101(box.maxPoint.x, box.minPoint.y, box.maxPoint.z);
	float3 p110(box.maxPoint.x, box.maxPoint.y, box.minPoint.z);
	float3 p111(box.maxPoint.x, box.maxPoint.y, box.maxPoint.z);
	Vertex vertices[36] =
	{
		{ p000, color, 0.f, 0.f }, { p001, color, 0.f, 0.f }, { p001, color, 0.f, 0.f },
		{ p001, color, 0.f, 0.f }, { p011, color, 0.f, 0.f }, { p011, color, 0.f, 0.f },
		{ p011, color, 0.f, 0.f }, { p010, color, 0.f, 0.f }, { p010, color, 0.f, 0.f },
		{ p010, color, 0.f, 0.f }, { p000, color, 0.f, 0.f }, { p000, color, 0.f, 0.f },

		{ p100, color, 0.f, 0.f }, { p101, color, 0.f, 0.f }, { p101, color, 0.f, 0.f },
		{ p101, color, 0.f, 0.f }, { p111, color, 0.f, 0.f }, { p111, color, 0.f, 0.f },
		{ p111, color, 0.f, 0.f }, { p110, color, 0.f, 0.f }, { p110, color, 0.f, 0.f },
		{ p110, color, 0.f, 0.f }, { p100, color, 0.f, 0.f }, { p100, color, 0.f, 0.f },

		{ p000, color, 0.f, 0.f }, { p100, color, 0.f, 0.f }, { p100, color, 0.f, 0.f },
		{ p001, color, 0.f, 0.f }, { p101, color, 0.f, 0.f }, { p101, color, 0.f, 0.f },
		{ p010, color, 0.f, 0.f }, { p110, color, 0.f, 0.f }, { p110, color, 0.f, 0.f },
		{ p011, color, 0.f, 0.f }, { p111, color, 0.f, 0.f }, { p111, color, 0.f, 0.f },
	};
	buffer->update(0, 36, vertices);
	video->draw(buffer, 0, 36, TOPOLOGY_TRIANGLELIST);
}

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
		pass->setFillMode(FILLMODE_WIREFRAME);
		pass->setCullMode(CULLMODE_NONE);
		pass->setDepthBias(-0.0003);
	}

	Color whiteData = 0xFFFFFFFF;
	whiteTexture = video->createTexture(1, 1, &whiteData);

	/* Create vertex format */
	vertexFormat = video->createVertexFormat();
	vertexFormat->addElement(TYPE_FLOAT3, SEMANTIC_POSITION);
	vertexFormat->addElement(TYPE_UBYTE4_NORM, SEMANTIC_COLOR);
	vertexFormat->addElement(TYPE_FLOAT2, SEMANTIC_TEXCOORD);

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
	while (device->beginFrame())
	{
		Event event;
		while (device->pollEvent(&event))
			/* Do nothing */;
		if (!device->getIsActive())
		{
			std::this_thread::yield();
			continue;
		}

		shortcutItemUI.update();
		World::CameraIntersectionInfo *info = nullptr;
		if (world->getCameraIntersection(Ray3D(camera->getPosition(), camera->getLookAt() - camera->getPosition()), &info))
		{
			if (device->isMousePressed(MOUSE_BUTTON_LEFT))
				info->block.setType(0);
			if (device->isMousePressed(MOUSE_BUTTON_RIGHT))
				info->block.getNeighbour(oppositeDirection(info->direction)).setType(shortcutItemUI.getCurrentItem());
		}
		while (device->tick())
		{
			world->tick();
			playerAnimator->tick();
		}
		world->update();
		world->save();

		playerAnimator->update();

		video->beginDraw(Color(127, 200, 251, 255));

		video->setMaterial(defaultMaterial);
		chunkSceneNode->render();
		if (world->getCameraIntersection(Ray3D(camera->getPosition(), camera->getLookAt() - camera->getPosition()), &info))
		{
			AABB3D box = info->block.getBoundingBox().translate(info->block.x(), info->block.y(), info->block.z());
			video->setMaterial(boxMaterial);
			draw3DBox(box, Color(255, 0, 0, 255));
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
