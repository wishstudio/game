#pragma once

#include <Chunk.h>

class WorldManipulator final
{
public:
	WorldManipulator(int _spanXMin, int _spanYMin, int _spanZMin, int _spanXMax, int _spanYMax, int _spanZMax) :
		spanXMin(_spanXMin), spanYMin(_spanYMin), spanZMin(_spanZMin),
		spanXMax(_spanXMax), spanYMax(_spanYMax), spanZMax(_spanZMax),
		sizeX(spanXMax - spanXMin + 1), sizeY(spanYMax - spanYMin + 1), sizeZ(spanZMax - spanZMin + 1)
	{
		chunks.reset(new (BlockData(*[sizeX * sizeY * sizeZ])[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]));
	}

	void addChunk(int rel_x, int rel_y, int rel_z, BlockData (*chunk)[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE])
	{
		chunks[(rel_x - spanXMin) * sizeY * sizeZ + (rel_y - spanYMin) * sizeZ + (rel_z - spanZMin)] = chunk;
	}

	class PlaneAccessor;
	inline const PlaneAccessor operator[] (int rel_x) const
	{
		return PlaneAccessor(this, rel_x);
	}

private:
	class StripAccessor;
	class PlaneAccessor final
	{
	public:
		PlaneAccessor(const WorldManipulator *_wm, int _rel_x) : wm(_wm), rel_x(_rel_x) {}
		inline const StripAccessor operator[] (int rel_y) const
		{
			return StripAccessor(wm, rel_x, rel_y);
		}

	private:
		const WorldManipulator *wm;
		int rel_x;
	};

	class StripAccessor final
	{
	public:
		StripAccessor(const WorldManipulator *_wm, int _rel_x, int _rel_y): wm(_wm), rel_x(_rel_x), rel_y(_rel_y) {}
		inline BlockData &operator[] (int rel_z) const
		{
			int cx = divide(rel_x, CHUNK_SIZE) - wm->spanXMin;
			int cy = divide(rel_y, CHUNK_SIZE) - wm->spanYMin;
			int cz = divide(rel_z, CHUNK_SIZE) - wm->spanZMin;
			return (*wm->chunks[cx * wm->sizeY * wm->sizeZ + cy * wm->sizeZ + cz])
				[mod(rel_x, CHUNK_SIZE)][mod(rel_y, CHUNK_SIZE)][mod(rel_z, CHUNK_SIZE)];
		}
		
	private:
		const WorldManipulator *wm;
		int rel_x, rel_y;
	};

	int spanXMin, spanYMin, spanZMin;
	int spanXMax, spanYMax, spanZMax;
	int sizeX, sizeY, sizeZ;
	std::unique_ptr<BlockData (*[])[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]> chunks;
};
