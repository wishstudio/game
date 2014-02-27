#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "IR/IRNode.h"
#include "IR/IRType.h"
#include "SymbolTable.h"

class Context
{
public:
	Context();

	SymbolTable symbolTable;
	std::vector<std::unique_ptr<IRType>> globalTypes;
	std::vector<std::unique_ptr<IRVariable>> globalVars;
	std::vector<std::unique_ptr<IRFunction>> globalFuncs;
};
