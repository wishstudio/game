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
	status = Status::Unloaded;

	this->chunk_x = chunk_x;
	this->chunk_y = chunk_y;
	this->chunk_z = chunk_z;

	dirty = true;
	bufferDirty = true;
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

void Chunk::loadAll()
{
	loadData();
	createMeshBuffer();
}

void Chunk::loadData()
{
	if (status >= Status::DataLoaded)
		return;
	status = Status::DataLoading;
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
	invalidateMeshBuffer();
	if (y + 1 == CHUNK_SIZE)
		world->getChunk(chunk_x, chunk_y + 1, chunk_z)->invalidateMeshBuffer();
	if (x + 1 == CHUNK_SIZE)
		world->getChunk(chunk_x + 1, chunk_y, chunk_z)->invalidateMeshBuffer();
	if (z + 1 == CHUNK_SIZE)
		world->getChunk(chunk_x, chunk_y, chunk_z + 1)->invalidateMeshBuffer();
	if (y == 0)
		world->getChunk(chunk_x, chunk_y - 1, chunk_z)->invalidateMeshBuffer();
	if (x == 0)
		world->getChunk(chunk_x - 1, chunk_y, chunk_z)->invalidateMeshBuffer();
	if (z == 0)
		world->getChunk(chunk_x, chunk_y, chunk_z - 1)->invalidateMeshBuffer();
}

void Chunk::invalidateMeshBuffer()
{
	status = Status::DataLoaded;
	bufferDirty = true;
}

void Chunk::OnRegisterSceneNode()
{
	if (IsVisible && status == Status::FullLoaded)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnRegisterSceneNode();
}

void Chunk::update()
{
	if (dirty)
		save();
	if (bufferDirty)
		createMeshBuffer();
}

void Chunk::render()
{
	update();
	driver->setTransform(ETS_WORLD, AbsoluteTransformation);
	for (u32 i = 0; i < collector.getBufferCount(); i++)
	{
		SMeshBuffer *buffer = collector.getBuffer(i);
		material.setTexture(0, collector.getBufferTexture(i));
		driver->setMaterial(material);
		driver->drawMeshBuffer(buffer);
	}
}

void Chunk::createMeshBuffer()
{
	if (status >= Status::FullLoaded)
		return;
	status = Status::BufferLoading;
	//Chunk *cy = mapManager->getChunk(chunk_x, chunk_y + 1, chunk_z);
	//Chunk *cx = mapManager->getChunk(chunk_x + 1, chunk_y, chunk_z);
	//Chunk *cz = mapManager->getChunk(chunk_x, chunk_y, chunk_z + 1);
	
	//Chunk *cmy = mapManager->getChunk(chunk_x, chunk_y - 1, chunk_z);
	//Chunk *cmx = mapManager->getChunk(chunk_x - 1, chunk_y, chunk_z);
	//Chunk *cmz = mapManager->getChunk(chunk_x, chunk_y, chunk_z - 1);
	
	collector.clear();
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				bool xCovered = x + 1 < CHUNK_SIZE && blockType->isCube(blocks[x + 1][y][z].type);
				bool yCovered = y + 1 < CHUNK_SIZE && blockType->isCube(blocks[x][y + 1][z].type);
				bool zCovered = z + 1 < CHUNK_SIZE && blockType->isCube(blocks[x][y][z + 1].type);
				bool mxCovered = x > 0 && blockType->isCube(blocks[x - 1][y][z].type);
				bool myCovered = y > 0 && blockType->isCube(blocks[x][y - 1][z].type);
				bool mzCovered = z > 0 && blockType->isCube(blocks[x][y][z - 1].type);
				Block block(this, x, y, z);
				blockType->drawBlock(&collector, block, xCovered, mxCovered, yCovered, myCovered, zCovered, mzCovered);
			}
	collector.finalize();
	bufferDirty = false;
	status = Status::FullLoaded;
}

s32 Chunk::getTriangleCount() const
{
	((Chunk *) this)->update();
	return collector.getTriangleCount();
}

/* Gets all triangles */
void Chunk::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const matrix4 *transform) const
{
	((Chunk *) this)->update();

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
	((Chunk *) this)->update();

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
	((Chunk *) this)->update();

	aabbox3df box(line.start);
	box.addInternalPoint(line.end);

	/* TODO: Could be optimized for line a little bit more. */
	getTriangles(triangles, arraySize, outTriangleCount, box, transform);
}
