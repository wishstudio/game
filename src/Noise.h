#pragma once

class Noise2D final
{
public:
	Noise2D(int seed, int octaveCount, float scale, float persistence);
	Noise2D(Noise2D &noise):
		sx(noise.sx), sy(noise.sy),
		spreadX(noise.spreadX), spreadY(noise.spreadY),
		seed(noise.seed), octaveCount(noise.octaveCount),
		persistence(noise.persistence), scale(noise.scale)
	{}

	Noise2D &operator= (const Noise2D &) = delete;

	void setSpread(float spreadX, float spreadY);
	
	void generate(int x, int y, int sx, int sy);
	inline float getNoise(int x, int y) const { return result[x * sy + y]; }

private:
	float noise(int seed, int x, int y) const;

	void generateOctave(int seed, float x, float y, float stepX, float stepY);

	std::unique_ptr<float[]> result, current;
	int sx, sy;
	float spreadX, spreadY;
	int seed, octaveCount;
	float persistence, scale;
};

class Noise3D final
{
public:
	Noise3D(int seed, int octaveCount, float scale, float persistence);
	Noise3D(Noise3D &noise):
		sx(noise.sx), sy(noise.sy), sz(noise.sz),
		spreadX(noise.spreadX), spreadY(noise.spreadY), spreadZ(noise.spreadZ),
		seed(noise.seed), octaveCount(noise.octaveCount),
		persistence(noise.persistence), scale(noise.scale)
	{}

	Noise3D &operator= (const Noise3D &) = delete;

	void setSpread(float spreadX, float spreadY, float spreadZ);

	void generate(int x, int y, int z, int sx, int sy, int sz);
	inline float getNoise(int x, int y, int z) const { return result[x * sy * sz + y * sz + z]; }

private:
	float noise(int seed, int x, int y, int z) const;

	void generateOctave(int seed, float x, float y, float z, float stepX, float stepY, float stepZ);
	
	std::unique_ptr<float[]> result, current;
	int sx, sy, sz;
	float spreadX, spreadY, spreadZ;
	int seed, octaveCount;
	float persistence, scale;
};
