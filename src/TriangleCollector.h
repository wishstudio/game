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
		const Vector3D &topLeft,
		const Color &topLeftColor,
		const Vector3D &topRight,
		const Color &topRightColor,
		const Vector3D &bottomLeft,
		const Color &bottomLeftColor,
		const Vector3D &bottomRight,
		const Color &bottomRightColor,
		const Vector3D &normal
	);

	/* Collision detection utilities */
	inline u32 getTriangleCount() const { return triangles.size(); }
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
	MeshBuffer *getBuffer(PTexture texture);
	void addTriangleIndex(MeshBuffer *buffer, u16 i1, u16 i2, u16 i3);

	bool valid;
	std::vector<MeshBuffer *> buffers;
	std::vector<PTexture> textures;
	std::unordered_map<PTexture, MeshBuffer *> textureMap;
	int x, y, z;
	std::vector<Triangle3D> triangles;
	int beginIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], endIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
