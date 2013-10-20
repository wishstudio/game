#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
#include "Noise.h"
#include "Serialization.h"
#include "World.h"

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

Chunk::Chunk(int chunk_x, int chunk_y, int chunk_z)
	: ISceneNode(smgr->getRootSceneNode(), smgr)
{
	status = Status::DataLoading;

	this->chunk_x = chunk_x;
	this->chunk_y = chunk_y;
	this->chunk_z = chunk_z;

	dirty = true;
	setPosition(vector3df(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE));

	boundingBox.reset(vector3df(0, 0, 0));
	boundingBox.addInternalPoint(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);

	material.Wireframe = false;
	material.Lighting = false;
}

Chunk::~Chunk()
{
}

void Chunk::initDatabase()
{
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS chunks (x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, data BLOB NOT NULL, PRIMARY KEY (x, y, z))", 0, 0, 0);
}

void Chunk::loadData()
{
	if (status >= Status::DataLoaded)
		return;
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT data FROM chunks WHERE x = ? AND y = ? AND z = ?", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, chunk_x);
	sqlite3_bind_int(stmt, 2, chunk_y);
	sqlite3_bind_int(stmt, 3, chunk_z);
	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		/* Not found, generate one */
		sqlite3_finalize(stmt);
		generate();
	}
	else
	{
		/* Found, read blob data */
		const void *data = sqlite3_column_blob(stmt, 0);
		u32 len = sqlite3_column_bytes(stmt, 0);
		Deserializer deserializer(data, len);
		for (int i = 0; i < CHUNK_SIZE; i++)
			for (int j = 0; j < CHUNK_SIZE; j++)
				for (int k = 0; k < CHUNK_SIZE; k++)
					deserializer >> blocks[i][j][k];
		sqlite3_finalize(stmt);
		dirty = false;
	}
	status = Status::DataLoaded;
}

void Chunk::save()
{
	if (!dirty || status < Status::DataLoaded)
		return;
	Serializer serializer;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			for (int k = 0; k < CHUNK_SIZE; k++)
				serializer << blocks[i][j][k];
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?)", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, chunk_x);
	sqlite3_bind_int(stmt, 2, chunk_y);
	sqlite3_bind_int(stmt, 3, chunk_z);
	sqlite3_bind_blob(stmt, 4, serializer.getData(), serializer.getLength(), free);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	dirty = false;
}

void Chunk::generate()
{
	memset(blocks, 0, sizeof blocks);
	Noise heightmap(1, 4, 20, 0.6);
	heightmap.setSize2D(CHUNK_SIZE, CHUNK_SIZE);
	heightmap.setSpread2D(180, 180);
	heightmap.generatePerlin2D(chunk_x * CHUNK_SIZE, chunk_z * CHUNK_SIZE);

	Noise noise(1, 4, 1, 0.4);
	noise.setSize3D(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
	noise.setSpread3D(5, 5, 5);
	noise.generatePerlin3D(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE);
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				int yy = chunk_y * CHUNK_SIZE + y;
				int h = heightmap.getNoise2D(x, z);
				if (yy < h)
				{
					float density = noise.getNoise3D(x, y, z);
					blocks[x][y][z].type = density > -0.4;
				}
			}
}

void Chunk::setDirty(int x, int y, int z)
{
	dirty = true;
	for (int offsetX = -1; offsetX <= 1; offsetX++)
		for (int offsetY = -1; offsetY <= 1; offsetY++)
			for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
			{
				Chunk *chunk = world->tryGetChunk(chunk_x + offsetX, chunk_y + offsetY, chunk_z + offsetZ);
				if (chunk)
					chunk->invalidateLight();
			}
}

void Chunk::invalidateLight()
{
	if (status >= Status::LightLoaded)
		status = Status::DataLoaded;
}

void Chunk::OnRegisterSceneNode()
{
	if (IsVisible && status >= Status::DataLoaded)
	{
		vector3df position = camera->getPosition();
		int x = (int) floor(position.X / CHUNK_SIZE);
		int y = (int) floor(position.Y / CHUNK_SIZE);
		int z = (int) floor(position.Z / CHUNK_SIZE);
		int dist = abs(chunk_x - x) + abs(chunk_y - y) + abs(chunk_z - z);

		if (dist <= 5)
		{
			if (status == Status::FullLoaded)
				SceneManager->registerNodeForRendering(this);
			else
				world->preloadChunkBuffer(this);
		}
	}

	ISceneNode::OnRegisterSceneNode();
}

void Chunk::render()
{
	driver->setTransform(ETS_WORLD, AbsoluteTransformation);
	for (u32 i = 0; i < collector.getBufferCount(); i++)
	{
		SMeshBuffer *buffer = collector.getBuffer(i);
		material.setTexture(0, collector.getBufferTexture(i));
		driver->setMaterial(material);
		driver->drawMeshBuffer(buffer);
	}
}

void Chunk::loadLight()
{
	if (status >= Status::LightLoaded)
		return;
	/* Clear sunlight */
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blocks[x][y][z].sunlight = 0;
	/* Propagate sunlight */
	if (chunk_y > 5)
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
		Chunk *chunk = world->tryGetChunk(chunk_x, chunk_y + 1, chunk_z);
		if (chunk == nullptr) /* Above chunk not exist, wait */
		{
			world->preloadChunk(chunk_x, chunk_y + 1, chunk_z);
			return;
		}
		if (chunk->getStatus() < Status::LightLoaded) /* Above chunk not ready, wait */
		{
			world->preloadChunkLight(chunk);
			return;
		}
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

	status = Status::LightLoaded;

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
		if (chunk && chunk->getStatus() >= Status::LightLoaded)
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
				dest.getChunk()->getStatus() >= Status::LightLoaded &&
				blockType->isLightTransparent(dest.getType()) &&
				light > dest.data->sunlight)
			{
				dest.data->sunlight = light;
				if (light > 1)
					queue.push(dest);
			}
		}
	}
}

void Chunk::loadBuffer()
{
	if (status >= Status::FullLoaded)
		return;

	for (int offsetX = -1; offsetX <= 1; offsetX++)
		for (int offsetY = -1; offsetY <= 1; offsetY++)
			for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
			{
				Chunk *chunk = world->preloadChunk(chunk_x + offsetX, chunk_y + offsetY, chunk_z + offsetZ);
				if (chunk->getStatus() < Status::LightLoaded)
				{
					world->preloadChunkLight(chunk);
					return;
				}
			}
	
	Chunk *cx = world->preloadChunk(chunk_x + 1, chunk_y, chunk_z);
	Chunk *cy = world->preloadChunk(chunk_x, chunk_y + 1, chunk_z);
	Chunk *cz = world->preloadChunk(chunk_x, chunk_y, chunk_z + 1);
	
	Chunk *cmx = world->preloadChunk(chunk_x - 1, chunk_y, chunk_z);
	Chunk *cmy = world->preloadChunk(chunk_x, chunk_y - 1, chunk_z);
	Chunk *cmz = world->preloadChunk(chunk_x, chunk_y, chunk_z - 1);
	
	collector.clear();
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				bool xCovered = blockType->isCube(x + 1 < CHUNK_SIZE?
					blocks[x + 1][y][z].type: cx->blocks[0][y][z].type);
				bool yCovered = blockType->isCube(y + 1 < CHUNK_SIZE?
					blocks[x][y + 1][z].type: cy->blocks[x][0][z].type);
				bool zCovered = blockType->isCube(z + 1 < CHUNK_SIZE?
					blocks[x][y][z + 1].type: cz->blocks[x][y][0].type);

				bool mxCovered = blockType->isCube(x > 0?
					blocks[x - 1][y][z].type: cmx->blocks[CHUNK_SIZE - 1][y][z].type);
				bool myCovered = blockType->isCube(y > 0?
					blocks[x][y - 1][z].type: cmy->blocks[x][CHUNK_SIZE - 1][z].type);
				bool mzCovered = blockType->isCube(z > 0?
					blocks[x][y][z - 1].type: cmz->blocks[x][y][CHUNK_SIZE - 1].type);

				u8 xLight = x + 1 < CHUNK_SIZE? blocks[x + 1][y][z].sunlight: cx->blocks[0][y][z].sunlight;
				u8 yLight = y + 1 < CHUNK_SIZE? blocks[x][y + 1][z].sunlight: cy->blocks[x][0][z].sunlight;
				u8 zLight = z + 1 < CHUNK_SIZE? blocks[x][y][z + 1].sunlight: cz->blocks[x][y][0].sunlight;

				u8 mxLight = x > 0? blocks[x - 1][y][z].sunlight: cmx->blocks[CHUNK_SIZE - 1][y][z].sunlight;
				u8 myLight = y > 0? blocks[x][y - 1][z].sunlight: cmy->blocks[x][CHUNK_SIZE - 1][z].sunlight;
				u8 mzLight = z > 0? blocks[x][y][z - 1].sunlight: cmz->blocks[x][y][CHUNK_SIZE - 1].sunlight;
				Block block(this, x, y, z);
				blockType->drawBlock(
					&collector, block,
					xCovered, mxCovered, yCovered, myCovered, zCovered, mzCovered,
					xLight, mxLight, yLight, myLight, zLight, mzLight
				);
			}
	collector.finalize();
	status = Status::FullLoaded;
}

s32 Chunk::getTriangleCount() const
{
	world->ensureChunkBufferLoaded((Chunk *) this);
	return collector.getTriangleCount();
}

/* Gets all triangles */
void Chunk::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const matrix4 *transform) const
{
	world->ensureChunkBufferLoaded((Chunk *) this);

	matrix4 mat;
	if (transform)
		mat = *transform;
	mat *= getAbsoluteTransformation();

	outTriangleCount = min(arraySize, getTriangleCount());
	for (auto it = collector.begin(); it != collector.end(); it++)
	{
		u32 i = it - collector.begin();
		mat.transformVect(triangles[i].pointA, it->pointA);
		mat.transformVect(triangles[i].pointB, it->pointB);
		mat.transformVect(triangles[i].pointC, it->pointC);
	}
}

/* Gets all triangles which have or may have contact within a specific bounding box */
void Chunk::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const aabbox3df &box,
	const matrix4 *transform) const
{
	world->ensureChunkBufferLoaded((Chunk *) this);

	matrix4 mat(matrix4::EM4CONST_NOTHING);
	aabbox3df tBox(box);
	getAbsoluteTransformation().getInverse(mat);
	mat.transformBoxEx(tBox);

	if (transform)
		mat = *transform;
	else
		mat.makeIdentity();
	mat *= getAbsoluteTransformation();

	s32 triangleCount = 0;

	int x_min = bound<int>(0, floor(tBox.MinEdge.X), CHUNK_SIZE - 1);
	int y_min = bound<int>(0, floor(tBox.MinEdge.Y), CHUNK_SIZE - 1);
	int z_min = bound<int>(0, floor(tBox.MinEdge.Z), CHUNK_SIZE - 1);
	int x_max = bound<int>(0, ceil(tBox.MaxEdge.X), CHUNK_SIZE - 1);
	int y_max = bound<int>(0, ceil(tBox.MaxEdge.Y), CHUNK_SIZE - 1);
	int z_max = bound<int>(0, ceil(tBox.MaxEdge.Z), CHUNK_SIZE - 1);
	for (int x = x_min; x <= x_max; x++)
		for (int y = y_min; y <= y_max; y++)
			for (int z = z_min; z <= z_max; z++)
			{
				for (auto it = collector.blockBegin(x, y, z); it != collector.blockEnd(x, y, z); it++)
				{
					mat.transformVect(triangles[triangleCount].pointA, it->pointA);
					mat.transformVect(triangles[triangleCount].pointB, it->pointB);
					mat.transformVect(triangles[triangleCount].pointC, it->pointC);
					triangleCount++;
					if (triangleCount == arraySize)
						goto done;
				}
			}

done:
	outTriangleCount = triangleCount;
}

/* Gets all triangles which have or may have contact with a 3d line */
void Chunk::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const line3df &line,
	const matrix4 *transform) const
{
	world->ensureChunkBufferLoaded((Chunk *) this);

	aabbox3df box(line.start);
	box.addInternalPoint(line.end);

	/* TODO: Could be optimized for line a little bit more. */
	getTriangles(triangles, arraySize, outTriangleCount, box, transform);
}
