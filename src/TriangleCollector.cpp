#include "stdafx.h"

#include "Block.h"
#include "TriangleCollector.h"
#include "World.h"

struct MeshBuffer
{
	PVertexBuffer vertexBuffer;
	PIndexBuffer indexBuffer;
	u32 indexCount = 0;
	std::vector<Vertex> vertices;
	std::vector<u16> indices;
};

TriangleCollector::TriangleCollector()
{
	valid = false;
}

TriangleCollector::~TriangleCollector()
{
	clear();
}

MeshBuffer *TriangleCollector::getBuffer(PTexture texture)
{
	auto it = textureMap.find(texture);
	if (it != textureMap.end())
		return it->second;
	else
	{
		MeshBuffer *buffer = new MeshBuffer();
		buffers.push_back(buffer);
		textures.push_back(texture);
		textureMap.insert(std::make_pair(texture, buffer));
		return buffer;
	}
}

void TriangleCollector::render()
{
	for (u32 i = 0; i < buffers.size(); i++)
	{
		MeshBuffer *buffer = buffers[i];
		PTexture texture = textures[i];
		if (buffer->vertexBuffer == nullptr)
		{
			buffer->vertexBuffer = video->createVertexBuffer(vertexFormat, buffer->vertices.size());
			buffer->vertexBuffer->update(0, buffer->vertices.size(), buffer->vertices.data());
			buffer->vertices.clear(); /* To save memory */
		}
		
		if (buffer->indexBuffer == nullptr)
		{
			buffer->indexBuffer = video->createIndexBuffer(TYPE_USHORT, buffer->indices.size());
			buffer->indexBuffer->update(0, buffer->indices.size(), buffer->indices.data());
			buffer->indexCount = buffer->indices.size();
			buffer->indices.clear(); /* To save memory */
		}
		video->setTexture(texture);
		video->drawIndexed(buffer->vertexBuffer, 0, buffer->indexBuffer, 0, buffer->indexCount, TOPOLOGY_TRIANGLELIST);
	}
}

void TriangleCollector::clear()
{
	for (MeshBuffer *buffer : buffers)
		delete buffer;
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
	const Vector3D &topLeft,
	const Color &topLeftColor,
	const Vector3D &topRight,
	const Color &topRightColor,
	const Vector3D &bottomLeft,
	const Color &bottomLeftColor,
	const Vector3D &bottomRight,
	const Color &bottomRightColor,
	const Vector3D &normal)
{
	MeshBuffer *buffer = getBuffer(tile.texture);

	u32 s = buffer->vertices.size();
	buffer->vertices.push_back({
		Vector3D(topLeft.x + x, topLeft.y + y, topLeft.z + z),
		topLeftColor,
		tile.u1, tile.v1
	});
	buffer->vertices.push_back({
		Vector3D(topRight.x + x, topRight.y + y, topRight.z + z),
		topRightColor,
		tile.u2, tile.v1
	});
	buffer->vertices.push_back({
		Vector3D(bottomLeft.x + x, bottomLeft.y + y, bottomLeft.z + z),
		bottomLeftColor,
		tile.u1, tile.v2
	});
	buffer->vertices.push_back({
		Vector3D(bottomRight.x + x, bottomRight.y + y, bottomRight.z + z),
		bottomRightColor,
		tile.u2, tile.v2
	});

	addTriangleIndex(buffer, s, s + 2, s + 1);
	addTriangleIndex(buffer, s + 1, s + 2, s + 3);
}

void TriangleCollector::addTriangleIndex(MeshBuffer *buffer, u16 i1, u16 i2, u16 i3)
{
	buffer->indices.push_back(i1);
	buffer->indices.push_back(i2);
	buffer->indices.push_back(i3);

	triangles.push_back(Triangle3D(
		buffer->vertices[i1].position,
		buffer->vertices[i2].position,
		buffer->vertices[i3].position
	));
}
