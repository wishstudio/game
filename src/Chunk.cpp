#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "Database.h"
#include "Serialization.h"
#include "TriangleCollector.h"
#include "World.h"
#include "WorldGenerator/WorldGenerator.h"
#include "WorldGenerator/WorldManipulator.h"

Serializer &operator << (Serializer &serializer, const BlockData &data)
{
	serializer << data.type << data.param1 << data.param2;
	return serializer;
}

Deserializer &operator >> (Deserializer &deserializer, BlockData &data)
{
	deserializer >> data.type >> data.param1 >> data.param2;
	return deserializer;
}

Chunk::Chunk(int _chunk_x, int _chunk_y, int _chunk_z)
	: chunk_x(_chunk_x), chunk_y(_chunk_y), chunk_z(_chunk_z), boundingBox({0, 0, 0})
{
	memset(blocks, 0, sizeof blocks);

	status = Status::Nothing;
	generationPhase = 0;
	inQueue = false;
	triangleCollector = nullptr;

	dirty = false;

	boundingBox.merge(Vector3D(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE));
	modelTransform = Matrix4::translation(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE);
}

Serializer &operator << (Serializer &serializer, const Chunk &data)
{
	if (data.getStatus() == Chunk::Status::Generating)
		serializer << data.generationPhase;
	else
		serializer << 0;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			for (int k = 0; k < CHUNK_SIZE; k++)
				serializer << data.blocks[i][j][k];
	return serializer;
}

Deserializer &operator >> (Deserializer &deserializer, Chunk &data)
{
	int generationPhase;
	deserializer >> generationPhase;
	if (generationPhase != 0)
	{
		data.status = Chunk::Status::Generating;
		data.generationPhase = generationPhase;
	}
	else
	{
		data.status = Chunk::Status::Data;
		data.generationPhase = WorldGenerator::getPhaseCount() + 1;
	}
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			for (int k = 0; k < CHUNK_SIZE; k++)
				deserializer >> data.blocks[i][j][k];
	return deserializer;
}

void Chunk::generate(int phase)
{
	if (generationPhase > phase)
		return;
	if (phase > 0)
	{
		/* Ensure surrounding chunks are all done [phase - 1] at least */
		WorldGenerator *generator = WorldGenerator::getGenerator(phase - 1);
		Vector3DI spanMin = generator->getSpanMin();
		Vector3DI spanMax = generator->getSpanMax();
		for (int x = spanMin.x; x <= spanMax.x; x++)
			for (int y = spanMin.y; y <= spanMax.y; y++)
				for (int z = spanMin.z; z <= spanMax.z; z++)
				{
					Chunk *chunk = world->rawGetChunk(chunk_x + x, chunk_y + y, chunk_z + z);
					chunk->loadRawData();
					if (chunk->generationPhase < phase)
						chunk->generate(phase - 1);
				}
	}

	if (phase == WorldGenerator::getPhaseCount())
	{
		/* We've done */
		std::lock_guard<std::mutex> lock(accessMutex);
		if (status == Status::Generating)
		{
			generationPhase = WorldGenerator::getPhaseCount() + 1;
			status = Status::Data;
		}
		return;
	}

	/* Lock related chunks for current phase */
	std::vector<std::unique_lock<std::mutex>> locks;
	WorldGenerator *generator = WorldGenerator::getGenerator(phase);
	Vector3DI spanMin = generator->getSpanMin();
	Vector3DI spanMax = generator->getSpanMax();
	WorldManipulator worldManipulator(spanMin.x, spanMin.y, spanMin.z, spanMax.x, spanMax.y, spanMax.z);
	for (int x = spanMin.x; x <= spanMax.x; x++)
		for (int y = spanMin.y; y <= spanMax.y; y++)
			for (int z = spanMin.z; z <= spanMax.z; z++)
			{
				Chunk *chunk = world->rawGetChunk(chunk_x + x, chunk_y + y, chunk_z + z);
				locks.push_back(std::unique_lock<std::mutex>(chunk->accessMutex));
				worldManipulator.addChunk(x, y, z, &chunk->blocks);
			}
	if (generationPhase > phase) /* Double check */
		return;

	/* Generate current phase */
	/* TODO: Seeding */
	generator->generate(1, chunk_x, chunk_y, chunk_z, worldManipulator);
	generationPhase = phase + 1;
}

void Chunk::setDirty(int x, int y, int z)
{
	if (!dirty)
		world->asyncSaveChunk(this);
	dirty = true;
	invalidateLight();
}

void Chunk::invalidateLight()
{
	status = Status::Data;
	world->asyncLoadChunk(this);
}

bool Chunk::shouldPreloadBuffer()
{
	return isInViewRange();
}

bool Chunk::isInViewRange()
{
	Vector3D position = camera->getPosition();
	int x = (int)floor(position.x / CHUNK_SIZE);
	int y = (int)floor(position.y / CHUNK_SIZE);
	int z = (int)floor(position.z / CHUNK_SIZE);
	int dist = abs(chunk_x - x) + abs(chunk_y - y) + abs(chunk_z - z);

	return dist <= 5;
}

void Chunk::render()
{
	if (!isInViewRange())
		return;

	TriangleCollector *collector = triangleCollector.load();
	if (collector == nullptr)
	{
		if (status < Status::Buffer)
			world->asyncLoadChunk(this);
		return;
	}
	video->setModelMatrix(modelTransform);
	collector->render();
}

void Chunk::save()
{
	if (!dirty || status < Status::Data)
		return;
	database->saveChunk(this);
	dirty = false;
}

/* Load chunk data from database, don't do anything else */
void Chunk::loadRawData()
{
	if (status >= Status::Generating)
		return;

	std::lock_guard<std::mutex> lock(accessMutex);
	if (status >= Status::Generating) /* Double check */
		return;

	if (!database->loadChunk(this))
	{
		/* Not found in database */
		status = Status::Generating;
		generationPhase = 0;
	}
}

void Chunk::loadData()
{
	if (status == Status::Generating)
	{
		generate(WorldGenerator::getPhaseCount());
		return;
	}
	loadRawData();
	if (status == Status::Generating)
		generate(WorldGenerator::getPhaseCount());
}

void Chunk::loadLight()
{
	if (status >= Status::Light)
		return;

	bool atTop = chunk_y > 5;
	if (!atTop)
	{
		Chunk *chunk = world->getChunk(chunk_x, chunk_y + 1, chunk_z);
		if (chunk->getStatus() < Status::Light)
			chunk->loadLight();
	}

	/* Lock surrounding chunks */
	std::vector<std::unique_lock<std::mutex>> locks;
	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{
				Chunk *chunk = world->tryGetChunk(chunk_x + x, chunk_y + y, chunk_z + z);
				if (chunk)
					locks.push_back(std::unique_lock<std::mutex>(chunk->accessMutex));
			}
	if (status >= Status::Light) /* Double check */
		return;

	/* Clear sunlight */
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blocks[x][y][z].sunlight = 0;
	/* Propagate sunlight */
	if (atTop)
	{
		/* Above chunk not exist */
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blocks[x][CHUNK_SIZE - 1][z].sunlight =
					blockType->isLightTransparent(blocks[x][CHUNK_SIZE - 1][z].type)? 15: 0;
	}
	else
	{
		/* Propagate from above chunk */
		Chunk *chunk = world->getChunk(chunk_x, chunk_y + 1, chunk_z);
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				if (blockType->isLightTransparent(blocks[x][CHUNK_SIZE - 1][z].type)
					&& chunk->blocks[x][0][z].sunlight == 15)
					blocks[x][CHUNK_SIZE - 1][z].sunlight = 15;
				else
					blocks[x][CHUNK_SIZE - 1][z].sunlight = 0;
	}
	/* Propagate top -> down */
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			for (int y = CHUNK_SIZE - 2; y >= 0; y--)
				if (blockType->isLightTransparent(blocks[x][y][z].type))
					blocks[x][y][z].sunlight = blocks[x][y + 1][z].sunlight;
				else
					blocks[x][y][z].sunlight = 0;

	/* Indirect illumination */
	Queue<Block> queue;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				if (blocks[x][y][z].sunlight > 1)
					queue.push(Block(this, x, y, z));
	/* Neighbour chunks */
	/* Order is consistent with Direction enum */
	int startX[DIRECTION_COUNT] = {              0,              0,              0, CHUNK_SIZE - 1,              0,              0};
	int   endX[DIRECTION_COUNT] = {              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	int startY[DIRECTION_COUNT] = {              0,              0,              0,              0, CHUNK_SIZE - 1,              0};
	int   endY[DIRECTION_COUNT] = { CHUNK_SIZE - 1,              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	int startZ[DIRECTION_COUNT] = {              0,              0,              0,              0,              0, CHUNK_SIZE - 1};
	int   endZ[DIRECTION_COUNT] = { CHUNK_SIZE - 1, CHUNK_SIZE - 1,              0, CHUNK_SIZE - 1, CHUNK_SIZE - 1, CHUNK_SIZE - 1};
	for (int i = 0; i < DIRECTION_COUNT; i++)
	{
		Chunk *chunk = world->tryGetChunk(chunk_x + dirX[i], chunk_y + dirY[i], chunk_z + dirZ[i]);
		if (chunk && chunk->getStatus() >= Status::Light)
		{
			for (int x = startX[i]; x <= endX[i]; x++)
				for (int y = startY[i]; y <= endY[i]; y++)
					for (int z = startZ[i]; z <= endZ[i]; z++)
						if (chunk->blocks[x][y][z].sunlight > 1)
							queue.push(Block(chunk, x, y, z));
		}
	}

	/* Calculate illumination */
	while (!queue.empty())
	{
		Block block = queue.pop();
		u8 light = block.data->sunlight - 1;
		for (int i = 0; i < DIRECTION_COUNT; i++)
		{
			Block dest = block.tryGetNeighbour((Direction) i);
			/* Ensure destination chunk has been lighted */
			if (dest.isValid() &&
				dest.getChunk()->getStatus() >= Status::Light &&
				blockType->isLightTransparent(dest.getType()) &&
				light > dest.data->sunlight)
			{
				dest.data->sunlight = light;
				if (light > 1)
					queue.push(dest);
			}
		}
	}

	status = Status::Light;
}

void Chunk::loadBuffer()
{
	if (status >= Status::Buffer)
		return;
	if (status < Status::Light)
		loadLight();

	for (int offsetX = -1; offsetX <= 1; offsetX++)
		for (int offsetY = -1; offsetY <= 1; offsetY++)
			for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
			{
				Chunk *chunk = world->getChunk(chunk_x + offsetX, chunk_y + offsetY, chunk_z + offsetZ);
				if (chunk->getStatus() < Status::Light)
					chunk->loadLight();
			}
	
	/* Double check */
	if (status >= Status::Buffer)
		return;
	TriangleCollector *collector = new TriangleCollector();
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blockType->drawBlock(collector, Block(this, x, y, z));
	collector->finalize();
	
	std::lock_guard<std::mutex> lock(accessMutex);
	TriangleCollector *oldCollector = triangleCollector.exchange(collector);
	if (oldCollector)
		world->asyncDeleteTriangleCollector(oldCollector);

	status = Status::Buffer;
}

/* Gets all triangles which have or may have contact within a specific bounding box */
void Chunk::getTriangles(std::vector<Triangle3D> &triangles, const AABB &box, const Matrix4 &transform)
{
	loadData();
	loadLight();
	loadBuffer();

	TriangleCollector *collector = triangleCollector.load();

	Matrix4 mat = modelTransform.getInverse();
	AABB tBox = box.transform(mat);

	mat = modelTransform * transform;

	int x_min = bound<int>(0, floor(tBox.minPoint.x), CHUNK_SIZE - 1);
	int y_min = bound<int>(0, floor(tBox.minPoint.y), CHUNK_SIZE - 1);
	int z_min = bound<int>(0, floor(tBox.minPoint.z), CHUNK_SIZE - 1);
	int x_max = bound<int>(0, ceil(tBox.maxPoint.x), CHUNK_SIZE - 1);
	int y_max = bound<int>(0, ceil(tBox.maxPoint.y), CHUNK_SIZE - 1);
	int z_max = bound<int>(0, ceil(tBox.maxPoint.z), CHUNK_SIZE - 1);
	for (int x = x_min; x <= x_max; x++)
		for (int y = y_min; y <= y_max; y++)
			for (int z = z_min; z <= z_max; z++)
			{
				for (auto it = collector->blockBegin(x, y, z); it != collector->blockEnd(x, y, z); it++)
				{
					triangles.push_back(Triangle3D(it->pointA * mat, it->pointB * mat, it->pointC * mat));
				}
			}
}
