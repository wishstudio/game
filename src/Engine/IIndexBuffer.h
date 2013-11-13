#pragma once
#pragma once

class IIndexBuffer
{
public:
	virtual VertexElementType getType() const = 0;
	virtual u32 getSize() const = 0;
	virtual void update(u32 startIndex, u32 index, const void *data) = 0;
};
