#include "stdafx.h"

#include "BlockType.h"
#include "Chunk.h"
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
	this->chunk_x = chunk_x;
	this->chunk_y = chunk_y;
	this->chunk_z = chunk_z;

	dirty = true;
	triangleSelectorDirty = true;
	bufferDirty = true;
	setPosition(vector3df(chunk_x * CHUNK_SIZE, chunk_y * CHUNK_SIZE, chunk_z * CHUNK_SIZE));

	boundingBox.reset(vector3df(0, 0, 0));
	boundingBox.addInternalPoint(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);

	material.Wireframe = false;
	material.Lighting = false;
	load();
}

Chunk::~Chunk()
{
}

void Chunk::initDatabase()
{
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS chunks (x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, data BLOB NOT NULL, PRIMARY KEY (x, y, z))", 0, 0, 0);
}

void Chunk::load()
{
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
}

void Chunk::generate()
{
	memset(blocks, 0, sizeof blocks);
	if (chunk_y >= 0)
		return;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				blocks[x][y][z].type = 1;
}

void Chunk::setDirty(int x, int y, int z)
{
	dirty = true;
	triangleSelectorDirty = true;
	bufferDirty = true;
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
	bufferDirty = true;
}

void Chunk::OnRegisterSceneNode()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnRegisterSceneNode();
}

void Chunk::render()
{
	if (dirty)
	{
		save();
		dirty = false;
	}
	if (bufferDirty)
	{
		createMeshBuffer();
		bufferDirty = false;
	}
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
}
