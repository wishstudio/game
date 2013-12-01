#include "stdafx.h"

#include "Noise.h"

#define NOISE_SEED_COEFFICIENT	797
#define NOISE_X_COEFFICIENT		1277
#define NOISE_Y_COEFFICIENT		7193
#define NOISE_Z_COEFFICIENT		14071

static f32 linearInterpolation(f32 p0, f32 p1, f32 t)
{
	return p0 + (p1 - p0) * t;
}

static f32 bilinearInterpolation(f32 p00, f32 p01, f32 p10, f32 p11, f32 x, f32 y)
{
	float r1 = linearInterpolation(p00, p10, x);
	float r2 = linearInterpolation(p01, p11, x);
	return linearInterpolation(r1, r2, y);
}

static f32 trilinearInterpolation(f32 p000, f32 p001, f32 p010, f32 p011,
	f32 p100, f32 p101, f32 p110, f32 p111,
	f32 x, f32 y, f32 z)
{
	f32 r00 = linearInterpolation(p000, p100, x);
	f32 r01 = linearInterpolation(p001, p101, x);
	f32 r10 = linearInterpolation(p010, p110, x);
	f32 r11 = linearInterpolation(p011, p111, x);
	return bilinearInterpolation(r00, r01, r10, r11, y, z);
}

Noise2D::Noise2D(int _seed, int _octaveCount, f32 _scale, f32 _persistence):
	seed(_seed), octaveCount(_octaveCount), scale(_scale), persistence(_persistence)
{
	sx = sy = 0;
}

void Noise2D::setSpread(f32 spreadX, f32 spreadY)
{
	this->spreadX = spreadX;
	this->spreadY = spreadY;
}

f32 Noise2D::noise(int seed, int x, int y) const
{
	int n = (seed * NOISE_SEED_COEFFICIENT + x * NOISE_X_COEFFICIENT + y * NOISE_Y_COEFFICIENT) & 0x7FFFFFFF;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
	return 1.f - (f32)n / 0x40000000;
}

void Noise2D::generateOctave(int seed, f32 x, f32 y, f32 stepX, f32 stepY)
{
	/* Calculate interpolated noise */
	int x0 = (int)floor(x), y0 = (int)floor(y);
	int index = 0;
	int cx = x0;
	f32 u = x - floor(x);
	for (int i = 0; i < sx; i++)
	{
		f32 v = y - floor(y);
		int cy = y0;

		f32 p00 = noise(seed, cx, cy);
		f32 p01 = noise(seed, cx, cy + 1);
		f32 p10 = noise(seed, cx + 1, cy);
		f32 p11 = noise(seed, cx + 1, cy + 1);
		for (int j = 0; j < sy; j++)
		{
			current[index++] = bilinearInterpolation(p00, p01, p10, p11, u, v);
			v += stepY;
			while (v >= 1.0)
			{
				v -= 1.0;
				cy++;
				p00 = p01;
				p01 = noise(seed, cx, cy + 1);
				p10 = p11;
				p11 = noise(seed, cx + 1, cy + 1);
			}
		}

		u += stepX;
		while (u >= 1.0)
		{
			u -= 1.0;
			cx++;
		}
	}
}

void Noise2D::generate(int x, int y, int _sx, int _sy)
{
	/* Allocate memory */
	sx = _sx, sy = _sy;
	int size = sx * sy;
	result.reset(new f32[size]);
	current.reset(new f32[size]);

	/* Calculate perlin octaves */
	memset(result.get(), 0, sizeof(float) * sx * sy);
	f32 frequency = 1, amplitude = 1;
	for (int oct = 0; oct < octaveCount; oct++)
	{
		/* Calculate and apply current octave */
		generateOctave(seed + oct, x * frequency / spreadX, y * frequency / spreadY, frequency / spreadX, frequency / spreadY);

		int index = 0;
		for (int i = 0; i < sx; i++)
		for (int j = 0; j < sy; j++)
		{
			result[index] += scale * amplitude * current[index];
			index++;
		}

		/* Update octave parameters */
		frequency *= 2;
		amplitude *= persistence;
	}
}

Noise3D::Noise3D(int _seed, int _octaveCount, f32 _scale, f32 _persistence):
	seed(_seed), octaveCount(_octaveCount), scale(_scale), persistence(_persistence)
{
	sx = sy = sz = 0;
}

void Noise3D::setSpread(f32 _spreadX, f32 _spreadY, f32 _spreadZ)
{
	spreadX = _spreadX, spreadY = _spreadY, spreadZ = _spreadZ;
}

f32 Noise3D::noise(int seed, int x, int y, int z) const
{
	int n = (seed * NOISE_SEED_COEFFICIENT + x * NOISE_X_COEFFICIENT + y * NOISE_Y_COEFFICIENT + z * NOISE_Z_COEFFICIENT) & 0x7FFFFFFF;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
	return 1.f - (f32) n / 0x40000000;
}

void Noise3D::generateOctave(int seed, f32 x, f32 y, f32 z, f32 stepX, f32 stepY, f32 stepZ)
{
	/* Calculate interpolated noise */
	int x0 = (int) floor(x), y0 = (int) floor(y), z0 = (int) floor(z);
	int index = 0;
	int cx = x0;
	f32 u = x - floor(x);
	for (int i = 0; i < sx; i++)
	{
		f32 v = y - floor(y);
		int cy = y0;
		for (int j = 0; j < sy; j++)
		{
			f32 w = z - floor(z);
			int cz = z0;
			f32 p000 = noise(seed, cx, cy, cz);
			f32 p001 = noise(seed, cx, cy, cz + 1);
			f32 p010 = noise(seed, cx, cy + 1, cz);
			f32 p011 = noise(seed, cx, cy + 1, cz + 1);
			f32 p100 = noise(seed, cx + 1, cy, cz);
			f32 p101 = noise(seed, cx + 1, cy, cz + 1);
			f32 p110 = noise(seed, cx + 1, cy + 1, cz);
			f32 p111 = noise(seed, cx + 1, cy + 1, cz + 1);
			for (int k = 0; k < sz; k++)
			{
				current[index++] = trilinearInterpolation(p000, p001, p010, p011, p100, p101, p110, p111, u, v, w);
				w += stepZ;
				while (w >= 1.0)
				{
					w -= 1.0;
					cz++;
					p000 = p001;
					p001 = noise(seed, cx, cy, cz + 1);
					p010 = p011;
					p011 = noise(seed, cx, cy + 1, cz + 1);
					p100 = p101;
					p101 = noise(seed, cx + 1, cy, cz + 1);
					p110 = p111;
					p111 = noise(seed, cx + 1, cy + 1, cz + 1);
				}
			}
			v += stepY;
			while (v >= 1.0)
			{
				v -= 1.0;
				cy++;
			}
		}
		u += stepX;
		while (u >= 1.0)
		{
			u -= 1.0;
			cx++;
		}
	}
}

void Noise3D::generate(int x, int y, int z, int _sx, int _sy, int _sz)
{
	/* Allocate memory */
	sx = _sx, sy = _sy, sz = _sz;
	int size = sx * sy * sz;
	result.reset(new f32[size]);
	current.reset(new f32[size]);

	/* Calculate perlin octaves */
	memset(result.get(), 0, sizeof(float) * sx * sy * sz);
	float frequency = 1, amplitude = 1;
	for (int oct = 0; oct < octaveCount; oct++)
	{
		/* Calculate and apply current octave */
		generateOctave(seed + oct, x * frequency / spreadX, y * frequency / spreadY, z * frequency / spreadZ,
			frequency / spreadX, frequency / spreadY, frequency / spreadZ);
		int index = 0;
		for (int i = 0; i < sx; i++)
			for (int j = 0; j < sy; j++)
				for (int k = 0; k < sz; k++)
				{
					result[index] += scale * amplitude * current[index];
					index++;
				}

		/* Update octave parameters */
		frequency *= 2;
		amplitude *= persistence;
	}
}
