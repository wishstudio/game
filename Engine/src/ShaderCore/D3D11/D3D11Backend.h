#pragma once

#include <string>

#include "../Context.h"

class D3D11Backend final
{
public:
	D3D11Backend() {}

	std::string getTypeName(IRType *type) const;
	void compileValue(IRValue *value);
	void compileSingleVariableDef(IRVariable *varDef);
	void compileNode(IRNode *node);
	std::string compile(Context *context);

private:
	Context *context;
	std::string ret;
};
