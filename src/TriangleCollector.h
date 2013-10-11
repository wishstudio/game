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
		const vector3df &normal
	);

private:
	SMeshBuffer *getBuffer(ITexture *texture);

	array<SMeshBuffer *> buffers;
	array<ITexture *> textures;
	std::unordered_map<ITexture *, SMeshBuffer *> textureMap;
	int x, y, z;
};
