#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "IR/IRNode.h"
#include "IR/IRType.h"

class Context
{
public:
	Context();

	std::vector<std::unique_ptr<IRType>> globalTypes;
	std::unordered_map<std::string, IRType *> types;
	std::vector<std::unique_ptr<IRVariableDef>> globalVars;
	std::vector<std::unique_ptr<IRFunction>> globalFuncs;
};
