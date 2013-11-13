#pragma once

class IVertexFormat;
class IVertexBuffer
{
public:
	virtual IVertexFormat *getVertexFormat() = 0;
	virtual u32 getSize() const = 0;
	virtual void update(u32 startIndex, u32 count, const void *data) = 0;
};
