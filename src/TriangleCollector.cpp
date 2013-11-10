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

MeshBuffer *TriangleCollector::getBuffer(ITexture *texture)
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
	const Vector3 &topLeft,
	const Color &topLeftColor,
	const Vector3 &topRight,
	const Color &topRightColor,
	const Vector3 &bottomLeft,
	const Color &bottomLeftColor,
	const Vector3 &bottomRight,
	const Color &bottomRightColor,
	const Vector3 &normal)
{
	MeshBuffer *buffer = getBuffer(tile.texture);

	u32 s = buffer->vertexBuffer.size();
	buffer->vertexBuffer.push_back({
		Vector3(topLeft.x + x, topLeft.y + y, topLeft.z + z),
		topLeftColor,
		tile.u1, tile.v1
	});
	buffer->vertexBuffer.push_back({
		Vector3(topRight.x + x, topRight.y + y, topRight.z + z),
		topRightColor,
		tile.u2, tile.v1
	});
	buffer->vertexBuffer.push_back({
		Vector3(bottomLeft.x + x, bottomLeft.y + y, bottomLeft.z + z),
		bottomLeftColor,
		tile.u1, tile.v2
	});
	buffer->vertexBuffer.push_back({
		Vector3(bottomRight.x + x, bottomRight.y + y, bottomRight.z + z),
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

	addTriangleIndex(buffer, s, s + 2, s + 1);
	addTriangleIndex(buffer, s + 1, s + 2, s + 3);
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
