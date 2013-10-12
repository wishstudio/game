#pragma once

class Noise
{
public:
	Noise(int seed, int octaveCount, float scale, float persistence);
	virtual ~Noise();

	void setSize2D(int sx, int sy);
	void setSize3D(int sx, int sy, int sz);
	void setSpread2D(float spreadX, float spreadY);
	void setSpread3D(float spreadX, float spreadY, float spreadZ);

	/* Perlin noise generators */
	void generatePerlin2D(int x, int y);
	void generatePerlin3D(int x, int y, int z);

	inline float getNoise2D(int x, int y) const { return result[x * sy + y]; }
	inline float getNoise3D(int x, int y, int z) const { return result[x * sy * sz + y * sz + z]; }

private:
	/* Interpolaton */
	float linearInterpolation(float p0, float p1, float t) const;
	float bilinearInterpolation(float p00, float p01, float p10, float p11, float x, float y) const;
	float trilinearInterpolation(
		float p000, float p001, float p010, float p011,
		float p100, float p101, float p110, float p111,
		float x, float y, float z) const;

	/* Noise functions */
	float noise2D(int seed, int x, int y) const;
	float noise3D(int seed, int x, int y, int z) const;

	/* Octave functions */
	void generateOctave2D(int seed, float x, float y, float stepX, float stepY);
	void generateOctave3D(int seed, float x, float y, float z, float stepX, float stepY, float stepZ);

	float *result, *current;
	int sx, sy, sz;
	int seed, octaveCount;
	float persistence, scale;
	float spreadX, spreadY, spreadZ;
};
