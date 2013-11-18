#pragma once

struct Tile
{
	Tile(): texture(nullptr) {}
	Tile(const Tile &another)
		: texture(another.texture), u1(another.u1), v1(another.v1), u2(another.u2), v2(another.v2)
	{
	}

	PTexture texture;
	float u1, v1, u2, v2;
};

class TileManager
{
public:
	TileManager();
	virtual ~TileManager();

	Tile registerTile(const std::string &resourcePath);
};
