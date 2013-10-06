#pragma once

class Chunk;
class ChunkTriangleSelector: public ITriangleSelector
{
public:
	ChunkTriangleSelector(Chunk *chunk);
	virtual ~ChunkTriangleSelector();

	Chunk *getChunk() const { return chunk; }

	/* ITriangleSelector */
	virtual ISceneNode *getSceneNodeForTriangle(u32 triangleIndex) const override;
	virtual ITriangleSelector *getSelector(u32 index) override { return this; }
	virtual const ITriangleSelector *getSelector(u32 index) const override { return this; }
	virtual u32 getSelectorCount() const { return 1; }
	virtual s32 getTriangleCount() const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const matrix4 *transform = nullptr
	) const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const aabbox3df &box,
		const matrix4 *transform = nullptr
	) const override;
	virtual void getTriangles(
		triangle3df *triangles,
		s32 arraySize,
		s32 &outTriangleCount,
		const line3df &line,
		const matrix4 *transform = nullptr
	) const override;

private:
	void update() const;

	mutable Chunk *chunk;
	/* Cached collision triangles */
	mutable array<triangle3df> triangles;
	/* Array offsets of each block in collision triangles cache */
	mutable int begin[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], end[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
