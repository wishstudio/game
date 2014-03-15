#pragma once

#include "../Context.h"

class D3D11ConstantReflection final
{
public:
	D3D11ConstantReflection(const std::string &_name, int _constantBufferSlot, int _offset, IRValueType *_type)
		: name(_name), constantBufferSlot(_constantBufferSlot), offset(_offset), type(_type) {}

	std::string getName() const { return name; }
	int getConstantBufferSlot() const { return constantBufferSlot; }
	int getOffset() const { return offset; }
	IRValueType *getType() const { return type; }

private:
	std::string name;
	int constantBufferSlot, offset;
	IRValueType *type;
};

class D3D11ConstantBufferReflection final
{
public:
	D3D11ConstantBufferReflection(int _slot) : slot(_slot), size(0) {}

	int getSize() const { return size; }
	int getConstantCount() const { return constants.size(); }
	D3D11ConstantReflection *getConstantReflection(int index) const { return constants[index].get(); }

private:
	friend class D3D11Backend;
	void addConstant(const std::string &name, IRValueType *type);

	int slot, size;
	std::vector<std::unique_ptr<D3D11ConstantReflection>> constants;
};
