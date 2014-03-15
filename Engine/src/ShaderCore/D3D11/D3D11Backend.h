#pragma once

#include <string>

#include "../Context.h"
#include "D3D11Reflection.h"

class D3D11Backend final
{
public:
	D3D11Backend() {}

	bool compile(const std::string &source);

	std::string getCompiledCode() const { return ret; }
	int getConstantBufferCount() const { return constantBuffers.size(); }
	D3D11ConstantBufferReflection *getConstantBufferReflection(int slot) const { return constantBuffers[slot].get(); }

private:
	std::string getTypeName(IRType *type) const;
	void compileValue(IRValue *value);
	void compileSingleVariableDef(IRVariable *varDef);
	void compileNode(IRNode *node);

	std::string ret;
	std::vector<std::unique_ptr<D3D11ConstantBufferReflection>> constantBuffers;
};
