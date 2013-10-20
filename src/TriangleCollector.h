#pragma once

#include "TileManager.h"

class Block;
class TriangleCollector
{
public:
	TriangleCollector();
	virtual ~TriangleCollector();

	inline u32 getBufferCount() const { return buffers.size(); }
	inline SMeshBuffer *getBuffer(u32 i) const { return buffers[i]; }
	inline ITexture *getBufferTexture(u32 i) const { return textures[i]; }
	
	void clear();
	void finalize();
	void setCurrentBlock(const Block &block);
	void addQuad(
		const Tile &tile,
		const vector3df &topLeft,
		const vector3df &topRight,
		const vector3df &bottomLeft,
		const vector3df &bottomRight,
		const vector3df &normal,
		const SColor &color
	);

	/* Collision detection utilities */
	inline u32 getTriangleCount() const { return triangles.size(); }
	inline std::vector<triangle3df>::const_iterator begin() const { return triangles.cbegin(); }
	inline std::vector<triangle3df>::const_iterator end() const { return triangles.cend(); }
	inline std::vector<triangle3df>::const_iterator blockBegin(int x, int y, int z) const
	{
		return triangles.cbegin() + beginIndex[x][y][z];
	}
	inline std::vector<triangle3df>::const_iterator blockEnd(int x, int y, int z) const
	{
		return triangles.cbegin() + endIndex[x][y][z];
	}

private:
	SMeshBuffer *getBuffer(ITexture *texture);
	void addTriangleIndex(SMeshBuffer *buffer, u32 i1, u32 i2, u32 i3);

	array<SMeshBuffer *> buffers;
	array<ITexture *> textures;
	std::unordered_map<ITexture *, SMeshBuffer *> textureMap;
	int x, y, z;
	std::vector<triangle3df> triangles;
	int beginIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], endIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
