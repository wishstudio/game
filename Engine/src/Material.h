#pragma once

#include <vector>

class Material
{
public:
	Material(Video *video);
	Material(const Material &) = delete;

	Material &operator= (const Material &) = delete;

	PPass createPass();
	u32 getPassCount() const { return passes.size(); }
	PPass getPass(u32 i) const { return passes[i]; }

private:
	Video *video;
	std::vector<PPass> passes;
};
