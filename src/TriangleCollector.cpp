#include "stdafx.h"

#include "Block.h"
#include "TriangleCollector.h"
#include "World.h"
#include "Engine/D3D11/D3D11Video.h"

TriangleCollector::TriangleCollector()
{
	valid = false;
}

TriangleCollector::~TriangleCollector()
{
	clear();
}

MeshBuffer *TriangleCollector::getBuffer(ITTexture *texture)
{
	auto it = textureMap.find(texture);
	if (it != textureMap.end())
		return it->second;
	else
	{
		MeshBuffer *buffer = video->createMeshBuffer();
		buffers.push_back(buffer);
		textures.push_back(texture);
		textureMap.insert(std::make_pair(texture, buffer));
		return buffer;
	}
}

void TriangleCollector::clear()
{
	for (MeshBuffer *buffer : buffers)
		video->deleteMeshBuffer(buffer);
	buffers.clear();
	textures.clear();
	textureMap.clear();
	triangles.clear();
	x = y = z = 0;
}

void TriangleCollector::finalize()
{
	endIndex[x][y][z] = triangles.size();
	valid = true;
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
	const SColor &topLeftColor,
	const vector3df &topRight,
	const SColor &topRightColor,
	const vector3df &bottomLeft,
	const SColor &bottomLeftColor,
	const vector3df &bottomRight,
	const SColor &bottomRightColor,
	const vector3df &normal)
{
	MeshBuffer *buffer = getBuffer(tile.texture);

	u32 s = buffer->vertexBuffer.size();
	buffer->vertexBuffer.push_back({
		vector3df(topLeft.X + x, topLeft.Y + y, topLeft.Z + z),
		topLeftColor,
		tile.u1, tile.v1
	});
	buffer->vertexBuffer.push_back({
		vector3df(topRight.X + x, topRight.Y + y, topRight.Z + z),
		topRightColor,
		tile.u2, tile.v1
	});
	buffer->vertexBuffer.push_back({
		vector3df(bottomLeft.X + x, bottomLeft.Y + y, bottomLeft.Z + z),
		bottomLeftColor,
		tile.u1, tile.v2
	});
	buffer->vertexBuffer.push_back({
		vector3df(bottomRight.X + x, bottomRight.Y + y, bottomRight.Z + z),
		bottomRightColor,
		tile.u2, tile.v2
	});
	/*u32 s = buffer->Vertices.size();
	buffer->Vertices.push_back(S3DVertex(
		topLeft.X + x, topLeft.Y + y, topLeft.Z + z,
		normal.X, normal.Y, normal.Z,
		topLeftColor,
		tile.u1, tile.v1
	));
	buffer->Vertices.push_back(S3DVertex(
		topRight.X + x, topRight.Y + y, topRight.Z + z,
		normal.X, normal.Y, normal.Z,
		topRightColor,
		tile.u2, tile.v1
	));
	buffer->Vertices.push_back(S3DVertex(
		bottomLeft.X + x, bottomLeft.Y + y, bottomLeft.Z + z,
		normal.X, normal.Y, normal.Z,
		bottomLeftColor,
		tile.u1, tile.v2
	));
	buffer->Vertices.push_back(S3DVertex(
		bottomRight.X + x, bottomRight.Y + y, bottomRight.Z + z,
		normal.X, normal.Y, normal.Z,
		bottomRightColor,
		tile.u2, tile.v2
	));*/

	addTriangleIndex(buffer, s, s + 1, s + 2);
	addTriangleIndex(buffer, s + 2, s + 1, s + 3);
}

void TriangleCollector::addTriangleIndex(MeshBuffer *buffer, u32 i1, u32 i2, u32 i3)
{
	buffer->indexBuffer.push_back(i1);
	buffer->indexBuffer.push_back(i2);
	buffer->indexBuffer.push_back(i3);

	triangles.push_back(triangle3df(
		buffer->vertexBuffer[i1].pos,
		buffer->vertexBuffer[i2].pos,
		buffer->vertexBuffer[i3].pos
	));
}
