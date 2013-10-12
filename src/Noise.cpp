#include "stdafx.h"

#include "Noise.h"

#define NOISE_SEED_COEFFICIENT	797
#define NOISE_X_COEFFICIENT		1277
#define NOISE_Y_COEFFICIENT		7193
#define NOISE_Z_COEFFICIENT		14071

Noise::Noise(int seed, int octaveCount, float scale, float persistence)
{
	this->seed = seed;
	this->octaveCount = octaveCount;
	this->scale = scale;
	this->persistence = persistence;
	
	result = nullptr;
	current = nullptr;
	sx = sy = sz = 0;
}

Noise::~Noise()
{
	if (result)
		delete result;
	if (current)
		delete current;
}

void Noise::setSize2D(int sx, int sy)
{
	setSize3D(sx, sy, 1);
}

void Noise::setSize3D(int sx, int sy, int sz)
{
	this->sx = sx;
	this->sy = sy;
	this->sz = sz;
	
	int size = sx * sy * sz;
	result = new float[size];
	current = new float[size];
}

void Noise::setSpread2D(float spreadX, float spreadY)
{
	setSpread3D(spreadX, spreadY, 1.0f);
}

void Noise::setSpread3D(float spreadX, float spreadY, float spreadZ)
{
	this->spreadX = spreadX;
	this->spreadY = spreadY;
	this->spreadZ = spreadZ;
}

float Noise::linearInterpolation(float p0, float p1, float t) const
{
	return p0 + (p1 - p0) * t;
}

float Noise::bilinearInterpolation(float p00, float p01, float p10, float p11, float x, float y) const
{
	float r1 = linearInterpolation(p00, p10, x);
	float r2 = linearInterpolation(p01, p11, x);
	return linearInterpolation(r1, r2, y);
}

float Noise::trilinearInterpolation(
	float p000, float p001, float p010, float p011,
	float p100, float p101, float p110, float p111,
	float x, float y, float z) const
{
	float r00 = linearInterpolation(p000, p100, x);
	float r01 = linearInterpolation(p001, p101, x);
	float r10 = linearInterpolation(p010, p110, x);
	float r11 = linearInterpolation(p011, p111, x);
	return bilinearInterpolation(r00, r01, r10, r11, y, z);
}

float Noise::noise2D(int seed, int x, int y) const
{
	int n = (seed * NOISE_SEED_COEFFICIENT + x * NOISE_X_COEFFICIENT + y * NOISE_Y_COEFFICIENT) & 0x7FFFFFFF;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
	return 1.f - (float) n / 0x40000000;
}

float Noise::noise3D(int seed, int x, int y, int z) const
{
	int n = (seed * NOISE_SEED_COEFFICIENT + x * NOISE_X_COEFFICIENT + y * NOISE_Y_COEFFICIENT + z * NOISE_Z_COEFFICIENT) & 0x7FFFFFFF;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF;
	return 1.f - (float) n / 0x40000000;
}

void Noise::generateOctave2D(int seed, float x, float y, float stepX, float stepY)
{
	/* Calculate interpolated noise */
	int x0 = (int) floor(x), y0 = (int) floor(y);
	int index = 0;
	int cx = x0;
	float u = x - floor(x);
	for (int i = 0; i < sx; i++)
	{
		float v = y - floor(y);
		int cy = y0;

		float p00 = noise2D(seed, cx, cy);
		float p01 = noise2D(seed, cx, cy + 1);
		float p10 = noise2D(seed, cx + 1, cy);
		float p11 = noise2D(seed, cx + 1, cy + 1);
		for (int j = 0; j < sy; j++)
		{
			current[index++] = bilinearInterpolation(p00, p01, p10, p11, u, v);
			v += stepY;
			while (v >= 1.0)
			{
				v -= 1.0;
				cy++;
				p00 = p01;
				p10 = p11;
				p01 = noise2D(seed, cx, cy + 1);
				p11 = noise2D(seed, cx + 1, cy + 1);
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

void Noise::generateOctave3D(int seed, float x, float y, float z, float stepX, float stepY, float stepZ)
{
}

void Noise::generatePerlin2D(int x, int y)
{
	/* Calculate perlin octaves */
	memset(result, 0, sizeof(float) * sx * sy);
	float frequency = 1, amplitude = 1;
	for (int oct = 0; oct < octaveCount; oct++)
	{
		/* Apply current octave */
		/* Step = 1 / T = 1 / (1 / f) = f */
		generateOctave2D(seed + oct, x * frequency / spreadX, y * frequency / spreadY, frequency / spreadX, frequency / spreadY);

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

void Noise::generatePerlin3D(int x, int y, int z)
{
}