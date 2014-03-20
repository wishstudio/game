#pragma once

#include "TileManager.h"

struct MeshBuffer;
class Block;
class TriangleCollector
{
public:
	TriangleCollector();
	virtual ~TriangleCollector();

	void render();
	
	bool isValid() { return valid; }
	void clear();
	void finalize();
	void setCurrentBlock(const Block &block);
	void addQuad(
		const Tile &tile,
		const float3 &topLeft,
		const Color &topLeftColor,
		const float3 &topRight,
		const Color &topRightColor,
		const float3 &bottomLeft,
		const Color &bottomLeftColor,
		const float3 &bottomRight,
		const Color &bottomRightColor,
		const float3 &normal
	);

	/* Collision detection utilities */
	inline int getTriangleCount() const { return triangles.size(); }
	inline std::vector<Triangle3D>::const_iterator begin() const { return triangles.cbegin(); }
	inline std::vector<Triangle3D>::const_iterator end() const { return triangles.cend(); }
	inline std::vector<Triangle3D>::const_iterator blockBegin(int x, int y, int z) const
	{
		return triangles.cbegin() + beginIndex[x][y][z];
	}
	inline std::vector<Triangle3D>::const_iterator blockEnd(int x, int y, int z) const
	{
		return triangles.cbegin() + endIndex[x][y][z];
	}

private:
	MeshBuffer *_getBuffer(PTexture texture);
	void _addTriangleIndex(MeshBuffer *buffer, int i1, int i2, int i3);

	bool valid;
	std::vector<MeshBuffer *> buffers;
	std::vector<PTexture> textures;
	std::unordered_map<PTexture, MeshBuffer *> textureMap;
	int x, y, z;
	std::vector<Triangle3D> triangles;
	int beginIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], endIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
