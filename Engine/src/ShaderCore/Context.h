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

	std::unordered_map<std::string, std::unique_ptr<IRType>> types;
	std::vector<std::unique_ptr<IRVariableDef>> globalVars;
};
