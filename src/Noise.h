#pragma once

class Noise2D final
{
public:
	Noise2D(int seed, int octaveCount, f32 scale, f32 persistence);
	Noise2D(Noise2D &noise):
		sx(noise.sx), sy(noise.sy),
		spreadX(noise.spreadX), spreadY(noise.spreadY),
		seed(noise.seed), octaveCount(noise.octaveCount),
		persistence(noise.persistence), scale(noise.scale)
	{}

	Noise2D &operator= (const Noise2D &) = delete;

	void setSpread(f32 spreadX, f32 spreadY);
	
	void generate(int x, int y, int sx, int sy);
	inline f32 getNoise(int x, int y) const { return result[x * sy + y]; }

private:
	f32 noise(int seed, int x, int y) const;

	void generateOctave(int seed, f32 x, f32 y, f32 stepX, f32 stepY);

	std::unique_ptr<f32[]> result, current;
	int sx, sy;
	f32 spreadX, spreadY;
	int seed, octaveCount;
	f32 persistence, scale;
};

class Noise3D final
{
public:
	Noise3D(int seed, int octaveCount, f32 scale, f32 persistence);
	Noise3D(Noise3D &noise):
		sx(noise.sx), sy(noise.sy), sz(noise.sz),
		spreadX(noise.spreadX), spreadY(noise.spreadY), spreadZ(noise.spreadZ),
		seed(noise.seed), octaveCount(noise.octaveCount),
		persistence(noise.persistence), scale(noise.scale)
	{}

	Noise3D &operator= (const Noise3D &) = delete;

	void setSpread(f32 spreadX, f32 spreadY, f32 spreadZ);

	void generate(int x, int y, int z, int sx, int sy, int sz);
	inline f32 getNoise(int x, int y, int z) const { return result[x * sy * sz + y * sz + z]; }

private:
	f32 noise(int seed, int x, int y, int z) const;

	void generateOctave(int seed, f32 x, f32 y, f32 z, f32 stepX, f32 stepY, f32 stepZ);
	
	std::unique_ptr<f32[]> result, current;
	int sx, sy, sz;
	f32 spreadX, spreadY, spreadZ;
	int seed, octaveCount;
	f32 persistence, scale;
};
