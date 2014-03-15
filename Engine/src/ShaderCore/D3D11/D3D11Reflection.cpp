#include "D3D11Reflection.h"

void D3D11ConstantBufferReflection::addConstant(const std::string &name, IRValueType *type)
{
	int s = type->getSize();
	/* Align to 4 byte boundary */
	s = (s + 3) & ~3;
	/* Don't cross 16 bytes boundary */
	if ((size & 15) + s >= 16)
		size = (size + 15) & ~15;
	constants.push_back(std::unique_ptr<D3D11ConstantReflection>(new D3D11ConstantReflection(name, slot, size, type)));
	size += s;
}
