#pragma once

class Texture
{
public:
	Texture() {}
	virtual ~Texture() {}

	virtual u32 getWidth() const = 0;
	virtual u32 getHeight() const = 0;
};
