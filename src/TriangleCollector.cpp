#include "stdafx.h"

#include "Block.h"
#include "TriangleCollector.h"

TriangleCollector::TriangleCollector()
{
}

TriangleCollector::~TriangleCollector()
{
	clear();
}

SMeshBuffer *TriangleCollector::getBuffer(ITexture *texture)
{
	auto it = textureMap.find(texture);
	if (it != textureMap.end())
		return it->second;
	else
	{
		SMeshBuffer *buffer = new SMeshBuffer();
		buffer->setHardwareMappingHint(EHM_DYNAMIC, EBT_VERTEX_AND_INDEX);
		buffers.push_back(buffer);
		textures.push_back(texture);
		textureMap.insert(std::make_pair(texture, buffer));
		return buffer;
	}
}

void TriangleCollector::clear()
{
	for (u32 i = 0; i < buffers.size(); i++)
		buffers[i]->drop();
	buffers.clear();
	textures.clear();
	textureMap.clear();
	triangles.clear();
	x = y = z = 0;
}

void TriangleCollector::finalize()
{
	endIndex[x][y][z] = triangles.size();
	for (u32 i = 0; i < buffers.size(); i++)
	{
		buffers[i]->setDirty();
		buffers[i]->recalculateBoundingBox();
	}
}

void TriangleCollector::setCurrentBlock(const Block &block)
{
	endIndex[x][y][z] = triangles.size();
	x = block.innerX();
	y = block.innerY();
	z = block.innerZ();
	beginIndex[x][y][z] = triangles.size();
}

void TriangleCollector::addQuad(
	const Tile &tile,
	const vector3df &topLeft,
	const vector3df &topRight,
	const vector3df &bottomLeft,
	const vector3df &bottomRight,
	const vector3df &normal)
{
	SMeshBuffer *buffer = getBuffer(tile.texture);
	const SColor w(255, 255, 255, 255);

	u32 s = buffer->Vertices.size();
	buffer->Vertices.push_back(S3DVertex(
		topLeft.X + x, topLeft.Y + y, topLeft.Z + z,
		normal.X, normal.Y, normal.Z,
		w,
		tile.u1, tile.v1
	));
	buffer->Vertices.push_back(S3DVertex(
		topRight.X + x, topRight.Y + y, topRight.Z + z,
		normal.X, normal.Y, normal.Z,
		w,
		tile.u2, tile.v1
	));
	buffer->Vertices.push_back(S3DVertex(
		bottomLeft.X + x, bottomLeft.Y + y, bottomLeft.Z + z,
		normal.X, normal.Y, normal.Z,
		w,
		tile.u1, tile.v2
	));
	buffer->Vertices.push_back(S3DVertex(
		bottomRight.X + x, bottomRight.Y + y, bottomRight.Z + z,
		normal.X, normal.Y, normal.Z,
		w,
		tile.u2, tile.v2
	));

	addTriangleIndex(buffer, s, s + 1, s + 2);
	addTriangleIndex(buffer, s + 2, s + 1, s + 3);
}

void TriangleCollector::addTriangleIndex(SMeshBuffer *buffer, u32 i1, u32 i2, u32 i3)
{
	buffer->Indices.push_back(i1);
	buffer->Indices.push_back(i2);
	buffer->Indices.push_back(i3);

	triangles.push_back(triangle3df(
		buffer->Vertices[i1].Pos,
		buffer->Vertices[i2].Pos,
		buffer->Vertices[i3].Pos
	));
}
