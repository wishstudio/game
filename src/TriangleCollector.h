#pragma once

#include "TileManager.h"

class Block;
class TriangleCollector
{
public:
	TriangleCollector();
	virtual ~TriangleCollector();

	inline u32 getBufferCount() const { return buffers.size(); }
	inline MeshBuffer *getBuffer(u32 i) const { return buffers[i]; }
	inline ITTexture *getBufferTexture(u32 i) const { return textures[i]; }
	
	bool isValid() { return valid; }
	void clear();
	void finalize();
	void setCurrentBlock(const Block &block);
	void addQuad(
		const Tile &tile,
		const vector3df &topLeft,
		const Color &topLeftColor,
		const vector3df &topRight,
		const Color &topRightColor,
		const vector3df &bottomLeft,
		const Color &bottomLeftColor,
		const vector3df &bottomRight,
		const Color &bottomRightColor,
		const vector3df &normal
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
	MeshBuffer *getBuffer(ITTexture *texture);
	void addTriangleIndex(MeshBuffer *buffer, u32 i1, u32 i2, u32 i3);

	bool valid;
	std::vector<MeshBuffer *> buffers;
	std::vector<ITTexture *> textures;
	std::unordered_map<ITTexture *, MeshBuffer *> textureMap;
	int x, y, z;
	std::vector<triangle3df> triangles;
	int beginIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], endIndex[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
