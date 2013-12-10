#pragma once

#include <vector>

class VertexFormat
{
public:
	virtual u32 getSize() const = 0;
	virtual void addElement(VertexElementType type, VertexElementSemantic semantic, u32 semanticIndex = 0) = 0;
};
