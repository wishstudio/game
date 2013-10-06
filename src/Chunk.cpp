#include "stdafx.h"

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
	material.setTexture(0, driver->getTexture("resources/stonepile.png"));
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
	driver->setMaterial(material);
	driver->setTransform(ETS_WORLD, AbsoluteTransformation);
	driver->drawMeshBuffer(&buffer);
}

void Chunk::createMeshBuffer()
{
	//Chunk *cy = mapManager->getChunk(chunk_x, chunk_y + 1, chunk_z);
	//Chunk *cx = mapManager->getChunk(chunk_x + 1, chunk_y, chunk_z);
	//Chunk *cz = mapManager->getChunk(chunk_x, chunk_y, chunk_z + 1);
	
	//Chunk *cmy = mapManager->getChunk(chunk_x, chunk_y - 1, chunk_z);
	//Chunk *cmx = mapManager->getChunk(chunk_x - 1, chunk_y, chunk_z);
	//Chunk *cmz = mapManager->getChunk(chunk_x, chunk_y, chunk_z - 1);
	buffer.setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX_AND_INDEX);
	buffer.Vertices.clear();
	buffer.Indices.clear();

	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				if (blocks[x][y][z].type == 0)
					continue;

				const vector3df p[8] = {
					vector3df(x + 0.f, y + 1.f, z + 0.f),
					vector3df(x + 0.f, y + 1.f, z + 1.f),
					vector3df(x + 1.f, y + 1.f, z + 0.f),
					vector3df(x + 1.f, y + 1.f, z + 1.f),
					vector3df(x + 0.f, y + 0.f, z + 0.f),
					vector3df(x + 0.f, y + 0.f, z + 1.f),
					vector3df(x + 1.f, y + 0.f, z + 0.f),
					vector3df(x + 1.f, y + 0.f, z + 1.f),
				};
				const vector3df nx(1, 0, 0);
				const vector3df nmx(-1, 0, 0);
				const vector3df ny(0, 1, 0);
				const vector3df nmy(0, -1, 0);
				const vector3df nz(0, 0, 1);
				const vector3df nmz(0, 0, -1);

				const SColor w(255, 255, 255, 255);

				const vector2df tex1(0, 1), tex2(0, 0), tex3(1, 1), tex4(1, 0);
				int s = buffer.Vertices.size();
				int count = 0;

				if (!(y + 1 < CHUNK_SIZE && blocks[x][y + 1][z].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[0], ny, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[1], ny, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[2], ny, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[3], ny, w, tex4));
					count++;
				}
				
				if (!(x + 1 < CHUNK_SIZE && blocks[x + 1][y][z].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[6], nx, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[2], nx, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[7], nx, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[3], nx, w, tex4));
					count++;
				}
				
				if (!(z + 1 < CHUNK_SIZE && blocks[x][y][z + 1].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[7], nz, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[3], nz, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[5], nz, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[1], nz, w, tex4));
					count++;
				}
				
				if (!(y > 0 && blocks[x][y - 1][z].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[5], nmy, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[4], nmy, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[7], nmy, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[6], nmy, w, tex4));
					count++;
				}
				
				if (!(x > 0 && blocks[x - 1][y][z].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[5], nmx, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[1], nmx, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[4], nmx, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[0], nmx, w, tex4));
					count++;
				}
				
				if (!(z > 0 && blocks[x][y][z - 1].type > 0))
				{
					buffer.Vertices.push_back(S3DVertex(p[4], nmz, w, tex1));
					buffer.Vertices.push_back(S3DVertex(p[0], nmz, w, tex2));
					buffer.Vertices.push_back(S3DVertex(p[6], nmz, w, tex3));
					buffer.Vertices.push_back(S3DVertex(p[2], nmz, w, tex4));
					count++;
				}
				
				for (int i = 0; i < count; i++)
				{
					buffer.Indices.push_back(s + 0);
					buffer.Indices.push_back(s + 1);
					buffer.Indices.push_back(s + 2);
					
					buffer.Indices.push_back(s + 2);
					buffer.Indices.push_back(s + 1);
					buffer.Indices.push_back(s + 3);

					s += 4;
				}
			}
	buffer.setDirty();
	buffer.recalculateBoundingBox();
}
