#include "stdafx.h"

#include "Chunk.h"
#include "ChunkTriangleSelector.h"

ChunkTriangleSelector::ChunkTriangleSelector(Chunk *chunk)
	: ITriangleSelector()
{
	this->chunk = chunk;
	this->chunk->triangleSelectorDirty = true;
}

ChunkTriangleSelector::~ChunkTriangleSelector()
{
}

ISceneNode *ChunkTriangleSelector::getSceneNodeForTriangle(u32 triangleIndex) const
{
	return chunk;
}

/* Update collision triangles cache */
void ChunkTriangleSelector::update() const
{
	if (!chunk->triangleSelectorDirty)
		return;

	chunk->triangleSelectorDirty = false;

	triangles.clear();
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				begin[x][y][z] = triangles.size();
				if (chunk->blocks[x][y][z].type != 0)
				{
					const vector3df p[8] = {
						vector3df(x + 0.f, y + 1.f, z + 0.f),
						vector3df(x + 0.f, y + 1.f, z + 1.f),
						vector3df(x + 1.f, y + 1.f, z + 0.f),
						vector3df(x + 1.f, y + 1.f, z + 1.f),
						vector3df(x + 0.f, y + 0.f, z + 0.f),
						vector3df(x + 0.f, y + 0.f, z + 1.f),
						vector3df(x + 1.f, y + 0.f, z + 0.f),
						vector3df(x + 1.f, y + 0.f, z + 1.f),
					};
				
					#define INSERT_SURFACE(t1, t2, t3, t4) \
						{ triangles.push_back(triangle3df(t1, t2, t3)); \
						triangles.push_back(triangle3df(t3, t2, t4)); }
					
					INSERT_SURFACE(p[0], p[1], p[2], p[3])
					INSERT_SURFACE(p[6], p[2], p[7], p[3])
					INSERT_SURFACE(p[7], p[3], p[5], p[1])
					INSERT_SURFACE(p[5], p[4], p[7], p[6])
					INSERT_SURFACE(p[5], p[1], p[4], p[0])
					INSERT_SURFACE(p[4], p[0], p[6], p[2])
				}
				end[x][y][z] = triangles.size();
			}
}

s32 ChunkTriangleSelector::getTriangleCount() const
{
	update();
	return triangles.size();
}

/* Gets all triangles */
void ChunkTriangleSelector::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const matrix4 *transform) const
{
	update();

	matrix4 mat;
	if (transform)
		mat = *transform;
	mat *= chunk->getAbsoluteTransformation();

	outTriangleCount = min(arraySize, getTriangleCount());
	for (s32 i = 0; i < outTriangleCount; i++)
	{
		mat.transformVect(triangles[i].pointA, this->triangles[i].pointA);
		mat.transformVect(triangles[i].pointB, this->triangles[i].pointB);
		mat.transformVect(triangles[i].pointC, this->triangles[i].pointC);
	}
}

/* Gets all triangles which have or may have contact within a specific bounding box */
void ChunkTriangleSelector::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const aabbox3df &box,
	const matrix4 *transform) const
{
	update();

	matrix4 mat(matrix4::EM4CONST_NOTHING);
	aabbox3df tBox(box);
	chunk->getAbsoluteTransformation().getInverse(mat);
	mat.transformBoxEx(tBox);

	if (transform)
		mat = *transform;
	else
		mat.makeIdentity();
	mat *= chunk->getAbsoluteTransformation();

	s32 triangleCount = 0;

	int x_min = bound<int>(0, floor(tBox.MinEdge.X), CHUNK_SIZE - 1);
	int y_min = bound<int>(0, floor(tBox.MinEdge.Y), CHUNK_SIZE - 1);
	int z_min = bound<int>(0, floor(tBox.MinEdge.Z), CHUNK_SIZE - 1);
	int x_max = bound<int>(0, ceil(tBox.MaxEdge.X), CHUNK_SIZE - 1);
	int y_max = bound<int>(0, ceil(tBox.MaxEdge.Y), CHUNK_SIZE - 1);
	int z_max = bound<int>(0, ceil(tBox.MaxEdge.Z), CHUNK_SIZE - 1);
	for (int x = x_min; x <= x_max; x++)
		for (int y = y_min; y <= y_max; y++)
			for (int z = z_min; z <= z_max; z++)
			{
				int s = begin[x][y][z], t = end[x][y][z];
				for (int i = s; i < t; i++)
				{
					mat.transformVect(triangles[triangleCount].pointA, this->triangles[i].pointA);
					mat.transformVect(triangles[triangleCount].pointB, this->triangles[i].pointB);
					mat.transformVect(triangles[triangleCount].pointC, this->triangles[i].pointC);
					triangleCount++;
					if (triangleCount == arraySize)
						goto done;
				}
			}

done:
	outTriangleCount = triangleCount;
}

/* Gets all triangles which have or may have contact with a 3d line */
void ChunkTriangleSelector::getTriangles(
	triangle3df *triangles,
	s32 arraySize,
	s32 &outTriangleCount,
	const line3df &line,
	const matrix4 *transform) const
{
	update();

	aabbox3df box(line.start);
	box.addInternalPoint(line.end);

	/* TODO: Could be optimized for line a little bit more. */
	getTriangles(triangles, arraySize, outTriangleCount, box, transform);
}
